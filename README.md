# lua和JavaScriptCore对比

## JavaScriptCore简介

- 开源JS引擎，Webkit在使用
- iOS 7.0及以上，提供Objective-C和C接口，不支持JIT

### 初始化

- JSGlobalContextRef可以理解为js的运行环境，所有的JS对象和执行状态都保存在这里
- 创建好之后可以通过JSContextGetGlobalObject获取全局对象

```c++
JSGlobalContextRef globalContext = JSGlobalContextCreate(nullptr);
JSObjectRef globalObject = JSContextGetGlobalObject(globalContext);
```

### 调用js函数

- 需要先将参数创建好放入数组arguments
- thisObject对应js中的this，为空则使用全局对象

```c++
JSValueRef JSObjectCallAsFunction(JSContextRef ctx, JSObjectRef object, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception);
```

### 注册js回调

js中函数也是first class，意味着我们可以像创建对象一样创建函数，然后通过JSObjectSetProperty设置到对应的对象中

```c++
JSValueRef CallbackFunc(JSContextRef ctx,
                        JSObjectRef function,
                        JSObjectRef thisObject,
                        size_t argumentCount,
                        const JSValueRef arguments[],
                        JSValueRef* exception)
{
    ...
}
JSStringRef funcName = JSStringCreateWithUTF8CString("func");
JSObjectRef funcObject = JSObjectMakeFunctionWithCallback(globalContext, funcName, &CallbackFunc);
JSObjectSetProperty(globalContext, globalObject, funcName, funcObject, kJSPropertyAttributeNone, nullptr);
```

### 创建JS类

- js类的的组织方式其实和lua类似，js对象有一个叫prototype的私有属性，当前对象没有的属性会在prototype表中查，因此可以模拟OOP的继承和多态等特性
- 对比上段的CallbackFunc定义我们可以看出，其实不管是类成员还是全局函数，其函数签名都是一样的，这是因为全局函数实际上也就是全局对象的一个函数类型（first class）的属性
- 所以这个创建类接口也只是一种方便快捷的做法，自己new一个object然后设置属性值也能达到一样的效果，只是相对繁琐

```c++
JSValueRef TestFunc(JSContextRef ctx,
                    JSObjectRef function,
                    JSObjectRef object,
                    size_t argumentCount,
                    const JSValueRef arguments[],
                    JSValueRef* exception)
{
    ...
}

JSObjectRef Constructor(JSContextRef ctx,
                        JSObjectRef object,
                        size_t argumentCount,
                        const JSValueRef arguments[],
                        JSValueRef* exception)
{
    ObjectType *obj = new ObjectType();
    JSObjectSetPrivate(object, reinterpret_cast<void*>(obj));
    return object;
}

void Destructor(JSObjectRef object)
{
    auto *obj = reinterpret_cast<ObjectType*>(JSObjectGetPrivate(object));
    delete fs;
}

JSClassDefinition claz = kJSClassDefinitionEmpty;
static JSStaticFunction staticFuncs[] = {
    { "testFunc", TestFunc, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { 0, 0, 0 }
};

static JSStaticValue staticValues[] = {
    { "testValue", getter, setter, kJSPropertyAttributeDontDelete },
    { 0, 0, 0, 0 }
};

claz.className = "TestClass";
claz.attributes = kJSClassAttributeNone;
claz.staticFunctions = staticFuncs;
claz.staticValues = staticValues;
claz.finalize = Destructor;
claz.callAsConstructor = Constructor;

testClass = JSClassCreate(&claz);
```

## lua简介

### lua库api对比

lua虚拟机和外界的数据交换通过栈来实现，栈底以1为起始索引，而栈顶则以-1作为起始索引，lua_gettop返回栈中元素个数

```lua
    |________|    <--  (-1)
    |________|    <--  (-2)
    |________|
    | ...... |
    |________|    <--  (2)
    |________|    <--  (1)
```

下面是lua和js的简单对比

| | lua | js |
| --- | --- | --- |
| 运行环境 | lua_state | JSGlobalContextRef |
| 全局对象获取 | _G表 | JSContextGetGlobalObject |
| 属性继承 | metatable | prototype |
| 对象析构 | __gc表 | finalize |
| 对象属性映射 | __newindex表 | JSClassDefinition.getPropertyNames |
| 函数调用 | stack | arguments数组 |

### 函数调用

函数调用这里lua和js有显著差异，需要先将函数压栈，然后将参数从左到右压栈

```c++
lua_getglobal(state, "add");
lua_pushnumber(state, 1);
lua_pushnumber(state, 2);
lua_call(state, 2, 1);
```

### 类以及对象

lua内部实际上没有对象机制，这里可以通过metatable来实现对象的属性和继承等，类似于js的prototype

- 建立lua table存放各个api函数，然后这个table以类名为key放在lua的registry表中，需要了解下js是否有这方面接口
- 代码内将metatale的__index函数和__newindex函数重定向到自己来实现查找，__gc实现对象销毁
    ```c++
    lua_getfield(luaState, -1, "__index");
    if (lua_isnil(luaState, -1))
    {
        lua_pushcfunction(luaState, &Index);
        lua_setfield(luaState, -3, "__index");
    }
    lua_pop(luaState, 1);

    lua_pushcfunction(luaState, &NewIndex);
    lua_setfield(luaState, -2, "__newindex");

    lua_pushcfunction(luaState, &GC);
    lua_setfield(luaState, -2, "__gc");

    lua_pushcfunction(luaState, &GetType);
    lua_setfield(luaState, -2, "GetType");
    ```
- 类的table中维护一个weak table，里面存放c++对象的指针，使得每一个c++对象对应唯一一个lua对象，这里js内部也可以参考类似实现，但是要考虑对象回收问题，避被根对象引用，影响垃圾回收
    ```c++
    lua_pushlightuserdata(luaState, pObj);
    lua_gettable(luaState, -2);
    if (lua_isnil(luaState, -1))
    {
        lua_pop(luaState, 1);
        Object** p = static_cast<Object**>(lua_newuserdata(luaState, sizeof(pObj)));
        *p = pObj;
        lua_pushlightuserdata(luaState, pObj);
        lua_pushvalue(luaState, -2);
        lua_settable(luaState, wt);
        lua_pushvalue(luaState, mt);
        lua_setmetatable(luaState, -2);
        lua_newtable(luaState);
        lua_setuservalue(luaState, -2);
    }
    ```

## 参考

[JavaScriptCore | Apple](https://developer.apple.com/documentation/javascriptcore)

[24.2 – The Stack | Lua](https://www.lua.org/pil/24.2.html)

[27.3.1 – The Registry | Lua](https://www.lua.org/pil/27.3.1.html)

[How to use the JavaScriptCore's C API](https://karhm.com/JavaScriptCore_C_API/)

[一个Javacript callback注册与调用的例子](http://blog.csdn.net/wowo1109/article/details/6715192)

[Taming JavascriptCore within and without WebView](http://parmanoir.com/Taming_JavascriptCore_within_and_without_WebView)