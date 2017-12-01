//
//  main.cpp
//  lua.test
//
//  Created by hongruichen on 2017/11/27.
//  Copyright © 2017年 hongruichen. All rights reserved.
//

#include <iostream>
#include <vector>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
using namespace std;

typedef lua_State* lua_State_Ptr;

namespace
{
    template <typename _Ty>
    class Allocator {};
    
    template<>
    class Allocator<lua_State_Ptr>
    {
    public:
        typedef lua_State_Ptr ObjectType;
        
        static ObjectType alloc() 
        {
            return luaL_newstate();
        }

        static void dealloc(ObjectType state)
        {
            lua_close(state);
        }
    };
    
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
                Allocator::dealloc(nullptr, m_object);
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
        lua_State_Ptr m_state;
        
    private:
        Handler(const Handler&) = delete;
        void operator=(const Handler&) = delete;
    };

    template <>
    class Handler<lua_State_Ptr>
    {
    public:
        typedef lua_State_Ptr ObjectType;
        typedef Allocator<lua_State_Ptr> Allocator;

        Handler()
        {
            m_object = Allocator::alloc();
        }

        Handler(Handler &&handler)
        {
            (*this) = std::move(handler);
        }

        virtual ~Handler()
        {
            if (m_object)
            {
                Allocator::dealloc(m_object);
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

    const char *kClassName = "TestClass";
}


int main(int argc, const char * argv[]) {
    std::cout << "lua.test.state.create" << endl;

    auto state = Handler<lua_State_Ptr>();
    const luaL_Reg loadedlibs[] = {
        { "_G", luaopen_base },
        { LUA_COLIBNAME, luaopen_coroutine },
        { LUA_TABLIBNAME, luaopen_table },
        { LUA_OSLIBNAME, luaopen_os },
        { LUA_STRLIBNAME, luaopen_string },
        { LUA_MATHLIBNAME, luaopen_math },
        { LUA_UTF8LIBNAME, luaopen_utf8 },
        { LUA_DBLIBNAME, luaopen_debug },
        { NULL, NULL }
    };

    const luaL_Reg *lib;
    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(state, lib->name, lib->func, 1);
        lua_pop(state, 1);
    }
    
    std::cout << "lua.test.start" << endl;
    {
        // call lua function
        luaL_dostring(state, R"--(function add(a, b) return a+b end)--");
        lua_getglobal(state, "add");
        lua_pushnumber(state, 1);
        lua_pushnumber(state, 2);
        lua_call(state, 2, 1);
        double result = lua_tonumber(state, lua_gettop(state));
        lua_pop(state, 1);
        cout << "call lua function add(1, 2), return " << result << endl;
    }
    {
        // regist callback function
        auto callback = [](lua_State_Ptr l) -> int
        {
            size_t count = lua_gettop(l);
            cout << "lua function callback, test(";
            for (size_t i = 1; i <= count; ++i)
            {
                if (i > 1)
                {
                    cout << ", ";
                }

                switch (lua_type(l, i))
                {
                    case LUA_TNONE:
                    case LUA_TNIL:
                        cout << "null";
                        break;
                    case LUA_TBOOLEAN:
                        cout << (lua_toboolean(l, i)? "true": "false");
                        break;
                    case LUA_TNUMBER:
                        cout << lua_tonumber(l, i);
                        break;
                }
            }
            cout << ")" << endl;
            lua_pop(l, count);
            return 0;
        };

        lua_pushcfunction(state, callback);
        lua_setglobal(state, "test");

        luaL_dostring(state, R"--(test(1, false))--");
    }
    {
        // lua class test
        auto Constructor = [](lua_State_Ptr l) -> int
        {
            auto ptr = (long*)lua_newuserdata(l, sizeof(long));
            *ptr = 123456;
            int newTable = lua_gettop(l);
            luaL_getmetatable(l, kClassName);
            lua_setmetatable(l, newTable);
            cout << "TestClass.constructor.call, data = " << *ptr << endl;
            return 1;
        };
        
        auto Destructor = [](lua_State_Ptr l) -> int
        {
            if (lua_type(l, 1) == LUA_TUSERDATA)
            {
                auto ptr = (long*)lua_touserdata(l, 1);
                cout << "TestClass.destructor.call, data = " << *ptr << endl;
            }
            else
            {
                cout << "TestClass.destructor.call, data.mismatch" << endl;
            }
            return 0;
        };

        auto Index = [](lua_State_Ptr l) -> int
        {
            if (lua_type(l, 1) != LUA_TUSERDATA)
            {
                cout << "TestClass.Index.call, data.mismatch" << endl;
                return 0;
            }
            lua_getmetatable(l, 1);
            lua_pushvalue(l, -2);
            lua_gettable(l, -2);
            return 1;
        };
        
        auto TestFunc = [](lua_State_Ptr l) -> int
        {
            if (lua_type(l, 1) == LUA_TUSERDATA)
            {
                auto ptr = (long*)lua_touserdata(l, 1);
                cout << "TestClass.testFunc.call, data = " << *ptr << endl;
            }
            else
            {
                cout << "TestClass.testFunc.call, data.mismatch" << endl;
            }
            lua_pushboolean(l, true);
            return 1;
        };
        
        int cnt = lua_gettop(state);
        luaL_newmetatable(state, kClassName);

        lua_pushcfunction(state, Index);
        lua_setfield(state, -2, "__index");
        cnt = lua_gettop(state);

        lua_pushcfunction(state, Destructor);
        lua_setfield(state, -2, "__gc");
        cnt = lua_gettop(state);

        lua_pushcfunction(state, TestFunc);
        lua_setfield(state, -2, "testFunc");
        cnt = lua_gettop(state);

        lua_pushcfunction(state, Constructor);
        lua_setfield(state, -2, "new");
        cnt = lua_gettop(state);

        lua_setglobal(state, kClassName);
        cnt = lua_gettop(state);

        int status = luaL_dostring(state, R"--(obj=TestClass.new();obj:testFunc();obj=nil)--");
        if (status != LUA_OK)
        {
            const char* szErrorMsg = lua_tostring(state, -1);
            cout << szErrorMsg << endl;
            lua_pop(state, 1);
        }
        lua_gc(state, LUA_GCCOLLECT, 0);
    }
    cout << "lua.test.end" << endl;

    cout << "lua.test.state.destroy" << endl;
    return 0;
}
