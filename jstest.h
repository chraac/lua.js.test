//
//  jstest.h
//
//  Created by hongruichen on 2017/11/27.
//  Copyright © 2017 hongruichen. All rights reserved.
//

#include <sstream>
#include <vector>
#include <JavaScriptCore/JavaScriptCore.h>


namespace JSTest
{
    template <typename _Ty>
    class Allocator {};
    
    template<>
    class Allocator<JSGlobalContextRef>
    {
    public:
        typedef JSGlobalContextRef ObjectType;
        
        static void unref(JSContextRef, ObjectType ctx)
        {
            JSGlobalContextRelease(ctx);
        }
    };
    
    template<>
    class Allocator<JSClassRef>
    {
    public:
        typedef JSClassRef ObjectType;
        
        static void unref(JSContextRef, ObjectType clz)
        {
            JSClassRelease(clz);
        }
    };
    
    template<>
    class Allocator<JSStringRef>
    {
    public:
        typedef JSStringRef ObjectType;
        
        static void unref(JSContextRef, ObjectType obj)
        {
            JSStringRelease(obj);
        }
    };
    
    template<>
    class Allocator<JSValueRef>
    {
    public:
        typedef JSValueRef ObjectType;
        
        static void ref(JSContextRef ctx, ObjectType obj)
        {
            JSValueProtect(ctx, obj);
        }
        
        static void unref(JSContextRef ctx, ObjectType obj)
        {
            if (ctx)
            {
                JSValueUnprotect(ctx, obj);
            }
        }
    };
    
    template<>
    class Allocator<JSObjectRef> : public Allocator<JSValueRef> {};
    
    template <typename _Ty, typename _Allocator = Allocator<_Ty>>
    class Handler
    {
    public:
        typedef _Ty ObjectType;
        typedef _Allocator Allocator;
        
        explicit Handler(ObjectType obj)
        : m_object(obj)
        {
        }
        
        Handler(Handler &&handler)
        {
            (*this) = std::move(handler);
        }
        
        virtual ~Handler()
        {
            if (m_object)
            {
                Allocator::unref(nullptr, m_object);
            }
        }
        
        ObjectType Get()const
        {
            return m_object;
        }
        
        void operator=(Handler &&handler)
        {
            m_object = handler.m_object;
            handler.m_object = nullptr;
        }
        
        operator ObjectType()const
        {
            return Get();
        }
        
        
    protected:
        ObjectType m_object;
        
    private:
        Handler(const Handler&) = delete;
        void operator=(const Handler&) = delete;
    };
    
    template<typename _Ty>
    class HandlerWithContext : public Handler<_Ty>
    {
    public:
        typedef _Ty ObjectType;
        typedef Handler<ObjectType> SuperClass;
        typedef typename SuperClass::Allocator Allocator;
        
        HandlerWithContext(JSContextRef ctx, ObjectType value)
        : Handler<ObjectType>(value)
        , m_context(ctx)
        {
            Allocator::ref(ctx, value);
            SuperClass::m_object = value;
        }
        
        HandlerWithContext(HandlerWithContext &&handler)
        : Handler<ObjectType>(nullptr)
        {
            (*this) = std::move(handler);
        }
        
        ~HandlerWithContext()
        {
            if (SuperClass::m_object)
            {
                Allocator::unref(m_context, SuperClass::m_object);
            }
        }
        
        void operator=(HandlerWithContext &&handler)
        {
            *static_cast<SuperClass*>(this) = std::move(handler);
            m_context = handler.m_context;
            handler.m_context = nullptr;
        }
        
    protected:
        JSContextRef m_context;
        
    private:
        HandlerWithContext(const HandlerWithContext&) = delete;
        void operator=(const HandlerWithContext&) = delete;
    };
    
    Handler<JSStringRef> MakeString(const char *sz)
    {
        return Handler<JSStringRef>(JSStringCreateWithUTF8CString(sz));
    }
    
    template<typename _Ty>
    HandlerWithContext<_Ty> MakeHandlerWithContext(JSContextRef ctx, _Ty value)
    {
        return HandlerWithContext<_Ty>(ctx, value);
    }
    
    
    const char *kClassName = "TestClass";
    class TestMain
    {
    public:
        static TestMain &GetInstance()
        {
            static TestMain instance;
            return instance;
        }
        
        void SetOutputStream(std::stringstream &ss)
        {
            m_ss = &ss;
        }
        
        std::stringstream &GetOutputStream()
        {
            return *m_ss;
        }
        
    private:
        std::stringstream *m_ss;
        
        TestMain() { m_ss = nullptr; }
        
        TestMain(TestMain&);
        TestMain(TestMain&&);
        void operator=(TestMain&);
        void operator=(TestMain&&);
    };
}


int main(int argc, const char * argv[]) {
    std::cout << "jscore.test.context.create" << endl;
    auto ctx = Handler<JSGlobalContextRef>(JSGlobalContextCreate(nullptr));
    
    std::cout << "jscore.test.start" << endl;
    JSValueRef exception = nullptr;
    {
        // call js function
        auto scriptString = MakeString(R"--(var add = function(a, b){return a+b;})--");
        JSEvaluateScript(ctx, scriptString, nullptr, nullptr, 0, &exception);
        {
            auto funcName = MakeString("add");
            auto obj = MakeHandlerWithContext(ctx, JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), funcName, &exception));
            JSGarbageCollect(ctx);
            auto func =  MakeHandlerWithContext(ctx, JSValueToObject(ctx, obj, &exception));
            JSGarbageCollect(ctx);
            JSValueRef args[2];
            args[0] = JSValueMakeNumber(ctx, 1.0);
            args[1] = JSValueMakeNumber(ctx, 2.0);
            JSValueRef ret = JSObjectCallAsFunction(ctx, func, nullptr, 2, args, &exception);
            cout << "call js function add(1, 2), return " << JSValueToNumber(ctx, ret, &exception) << endl;
        }
    }
    {
        // regist callback function
        auto callback = [](JSContextRef ctx,
                           JSObjectRef function,
                           JSObjectRef thisObject,
                           size_t argumentCount,
                           const JSValueRef arguments[],
                           JSValueRef* exception) -> JSValueRef
        {
            cout << "js function callback, test(";
            for (size_t i = 0; i < argumentCount; ++i)
            {
                if (i > 0)
                {
                    cout << ", ";
                }
                auto &arg = arguments[i];
                JSType type = JSValueGetType(ctx, arg);
                switch (type)
                {
                    case kJSTypeNull:
                        cout << "null";
                        break;
                    case kJSTypeBoolean:
                        cout << (JSValueToBoolean(ctx, arg)? "true": "false");
                        break;
                    case kJSTypeNumber:
                        cout << JSValueToNumber(ctx, arg, exception);
                        break;
                }
            }
            cout << ")" << endl;
            return JSValueMakeUndefined(ctx);
        };
        
        auto funcName = MakeString("test");
        JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx, funcName, callback);
        JSGarbageCollect(ctx);
        JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), funcName, func, kJSPropertyAttributeNone, &exception);
        
        auto scriptString = MakeString(R"--(test(1, false);)--");
        JSEvaluateScript(ctx, scriptString, nullptr, nullptr, 0, &exception);
    }
    {
        // js class test
        auto Constructor = [] (JSContextRef ctx,
                               JSObjectRef object,
                               size_t argumentCount,
                               const JSValueRef arguments[],
                               JSValueRef* exception) -> JSObjectRef
        {
            long data = 123456;
            JSObjectSetPrivate(object, reinterpret_cast<void*>(data));
            cout << "TestClass.constructor.call, data = " << data << endl;
            return object;
        };
        
        auto Destructor = [] (JSObjectRef object)
        {
            auto data = (long)JSObjectGetPrivate(object);
            cout << "TestClass.destructor.call, data = " << data << endl;
        };
        
        auto TestFunc = [] (JSContextRef ctx,
                            JSObjectRef function,
                            JSObjectRef thisObject,
                            size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef* exception) -> JSValueRef
        {
            cout << "TestClass.testFunc.call" << endl;
            return JSValueMakeBoolean(ctx, true);
        };
        
        auto getter = [] (JSContextRef ctx,
                          JSObjectRef object,
                          JSStringRef propertyName,
                          JSValueRef* exception) -> JSValueRef
        {
            vector<char> buffer(JSStringGetMaximumUTF8CStringSize(propertyName));
            JSStringGetUTF8CString(propertyName, buffer.data(), buffer.size());
            cout << "TestClass." << buffer.data() << ".getter.call" << endl;
            return JSValueMakeNumber(ctx, 1.23456);
        };
        
        auto setter = [] (JSContextRef ctx,
                          JSObjectRef object,
                          JSStringRef propertyName,
                          JSValueRef value,
                          JSValueRef* exception) -> bool
        {
            vector<char> buffer(JSStringGetMaximumUTF8CStringSize(propertyName));
            JSStringGetUTF8CString(propertyName, buffer.data(), buffer.size());
            cout << "TestClass." << buffer.data() << ".setter.call.";
            cout << JSValueToNumber(ctx, value, exception) << endl;
            return true;
        };
        
        JSClassDefinition clzDef = kJSClassDefinitionEmpty;
        static JSStaticFunction staticFuncs[] = {
            { "testFunc", TestFunc, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
            { 0, 0, 0 }
        };
        
        static JSStaticValue staticValues[] = {
            { "testValue", getter, setter, kJSPropertyAttributeDontDelete },
            { 0, 0, 0, 0 }
        };
        
        clzDef.className = "TestClass";
        clzDef.attributes = kJSClassAttributeNone;
        clzDef.staticFunctions = staticFuncs;
        clzDef.staticValues = staticValues;
        clzDef.finalize = Destructor;
        clzDef.callAsConstructor = Constructor;
        
        {
            auto clz = Handler<JSClassRef>(JSClassCreate(&clzDef));
            auto clzName = MakeString("TestClass");
            JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), clzName, JSObjectMake(ctx, clz, nullptr), kJSPropertyAttributeNone, &exception);
            JSGarbageCollect(ctx);
        }
        
        auto scriptString = MakeString(R"--(var tst = new TestClass();tst.testValue = 789;var t = tst.testValue;tst.testFunc();)--");
        JSEvaluateScript(ctx, scriptString, nullptr, nullptr, 0, &exception);
    }
    std::cout << "jscore.test.end" << endl;
    
    std::cout << "jscore.test.context.destroy" << endl;
    return 0;
}
