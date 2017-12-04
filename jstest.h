//
//  jstest.h
//
//  Created by hongruichen on 2017/11/27.
//  Copyright © 2017 hongruichen. All rights reserved.
//

#include <sstream>
#include <vector>
#include <JavaScriptCore/JavaScriptCore.h>
#include "testcommon.h"
#include "handler.h"


namespace JSTest
{
    TestCommon::Handler<JSStringRef> MakeString(const char *sz)
    {
        return TestCommon::Handler<JSStringRef>(JSStringCreateWithUTF8CString(sz));
    }
    
    template<typename _Ty>
    TestCommon::Handler<_Ty> MakeHandlerWithContext(JSContextRef ctx, _Ty value)
    {
        return TestCommon::Handler<_Ty>(ctx, value);
    }
    
    const char *kClassName = "TestClass";
    class TestMain : public TestCommon::TestBase<TestMain>
    {
    public:
        TestMain() {}
        
        void RunAllTest(TestEvent start, TestEvent end)
        {
            GetOutputStream() << "jscore.test.context.create" << std::endl;
            if (start)
            {
                start();
            }
            auto ctx = TestCommon::Handler<JSGlobalContextRef>();
            GetOutputStream() << "jscore.test.start" << std::endl;
            RunCallFunctionTest(ctx);
            RunFunctionCallbackTest(ctx);
            RunClassTest(ctx);
            if (end)
            {
                end();
            }
            GetOutputStream() << "jscore.test.end" << std::endl;
            
            GetOutputStream() << "jscore.test.context.destroy" << std::endl;
        }
        
    private:
        void RunCallFunctionTest(JSGlobalContextRef ctx)
        {
            // call js function
            JSValueRef exception = nullptr;
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
                GetOutputStream() << "call js function add(1, 2), return " << JSValueToNumber(ctx, ret, &exception) << std::endl;
            }
        }
        
        void RunFunctionCallbackTest(JSGlobalContextRef ctx)
        {
            // regist callback function
            JSValueRef exception = nullptr;
            auto callback = [](JSContextRef ctx,
                               JSObjectRef function,
                               JSObjectRef thisObject,
                               size_t argumentCount,
                               const JSValueRef arguments[],
                               JSValueRef* exception) -> JSValueRef
            {
                if (argumentCount != 2) {
                    return JSValueMakeUndefined(ctx);
                }
                return JSValueMakeNumber(ctx, JSValueToNumber(ctx, arguments[0], exception) - JSValueToNumber(ctx, arguments[1], exception));
            };
            
            auto funcName = MakeString("sub");
            JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx, funcName, callback);
            JSGarbageCollect(ctx);
            JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), funcName, func, kJSPropertyAttributeNone, &exception);
            
            auto scriptString = MakeString(R"--(sub(1, 2);)--");
            JSValueRef res = JSEvaluateScript(ctx, scriptString, nullptr, nullptr, 0, &exception);
            GetOutputStream() << "js function callback sub(1, 2), return " << JSValueToNumber(ctx, res, &exception) << std::endl;
        }
        
        void RunClassTest(JSGlobalContextRef ctx)
        {
            // js class test
            JSValueRef exception = nullptr;
            auto Constructor = [] (JSContextRef ctx,
                                   JSObjectRef object,
                                   size_t argumentCount,
                                   const JSValueRef arguments[],
                                   JSValueRef* exception) -> JSObjectRef
            {
                long data = 123456;
                JSObjectSetPrivate(object, reinterpret_cast<void*>(data));
                TestMain::GetInstance().GetOutputStream() << "TestClass.constructor.call, data = " << data << std::endl;
                return object;
            };
            
            auto Destructor = [] (JSObjectRef object)
            {
                auto data = (long)JSObjectGetPrivate(object);
                TestMain::GetInstance().GetOutputStream() << "TestClass.destructor.call, data = " << data << std::endl;
            };
            
            auto TestFunc = [] (JSContextRef ctx,
                                JSObjectRef function,
                                JSObjectRef thisObject,
                                size_t argumentCount,
                                const JSValueRef arguments[],
                                JSValueRef* exception) -> JSValueRef
            {
                TestMain::GetInstance().GetOutputStream() << "TestClass.testFunc.call" << std::endl;
                return JSValueMakeBoolean(ctx, true);
            };
            
            JSClassDefinition clzDef = kJSClassDefinitionEmpty;
            static JSStaticFunction staticFuncs[] = {
                { "testFunc", TestFunc, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
                { 0, 0, 0 }
            };
            
            static JSStaticValue staticValues[] = {
                { 0, 0, 0, 0 }
            };
            
            clzDef.className = "TestClass";
            clzDef.attributes = kJSClassAttributeNone;
            clzDef.staticFunctions = staticFuncs;
            clzDef.staticValues = staticValues;
            clzDef.finalize = Destructor;
            clzDef.callAsConstructor = Constructor;
            
            {
                auto clz = TestCommon::Handler<JSClassRef>(JSClassCreate(&clzDef));
                auto clzName = MakeString("TestClass");
                JSObjectSetProperty(ctx, JSContextGetGlobalObject(ctx), clzName, JSObjectMake(ctx, clz, nullptr), kJSPropertyAttributeNone, &exception);
                JSGarbageCollect(ctx);
            }
            
            auto scriptString = MakeString(R"--(var tst = new TestClass();tst.testFunc();tst=null;)--");
            JSEvaluateScript(ctx, scriptString, nullptr, nullptr, 0, &exception);
            JSGarbageCollect(ctx);
        }
        
        DISALLOW_COPY(TestMain);
        DISALLOW_MOVE(TestMain);
    };
}
