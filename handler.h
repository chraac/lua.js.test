//
//  handler.h
//
//  Created by hongruichen on 2017/11/27.
//  Copyright © 2017 hongruichen. All rights reserved.
//

#ifndef __HANDLER_H__
#define __HANDLER_H__

#include "testcommon.h"
#include "lua.h"

namespace TestCommon
{
    template <typename _Ty>
    class Allocator {};
    
    template<>
    class Allocator<lua_State*>
    {
    public:
        typedef lua_State* ObjectType;
        static constexpr bool HasContext = false;
        static constexpr bool HasAllocator = true;
        
        static ObjectType alloc()
        {
            return luaL_newstate();
        }
        
        static void dealloc(ObjectType state)
        {
            lua_close(state);
        }
    };
    
    template <typename _Ty,
        bool _hasContext = Allocator<_Ty>::HasContext,
        bool _hasAllocator = Allocator<_Ty>::HasAllocator>
    class Handler
    {
    public:
        typedef _Ty ObjectType;
        typedef Allocator<_Ty> Allocator;
        typedef typename Allocator::ContextType ContextType;
        
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
                Allocator::dealloc(m_context, m_object);
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
        ContextType m_context;
        
    private:
        Handler(const Handler&) = delete;
        void operator=(const Handler&) = delete;
    };
    
    template <typename _Ty>
    class Handler<_Ty, false, true>
    {
    public:
        typedef _Ty ObjectType;
        typedef Allocator<_Ty> Allocator;
        
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
}

#endif
