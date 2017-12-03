//
//  handler.h
//
//  Created by hongruichen on 2017/11/27.
//  Copyright © 2017 hongruichen. All rights reserved.
//

#ifndef __HANDLER_H__
#define __HANDLER_H__

#include <JavaScriptCore/JavaScriptCore.h>
#include "testcommon.h"
#include "lua.h"

#define COMMON_MOVE_FUNCTIONS(clz, super) \
    clz(clz &&obj) \
    { \
        (*this) = std::move(obj); \
    } \
    void operator=(clz &&obj) \
    { \
        *((super*)this) = std::move(obj); \
    }


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
    
    template<>
    class Allocator<JSGlobalContextRef>
    {
    public:
        typedef JSGlobalContextRef ObjectType;
        static constexpr bool HasContext = false;
        static constexpr bool HasAllocator = true;
        
        static ObjectType alloc()
        {
            return JSGlobalContextCreate(nullptr);
        }
        
        static void dealloc(ObjectType ctx)
        {
            JSGlobalContextRelease(ctx);
        }
    };
    
    template<>
    class Allocator<JSClassRef>
    {
    public:
        typedef JSClassRef ObjectType;
        static constexpr bool HasContext = false;
        static constexpr bool HasAllocator = false;
        
        static void dealloc(ObjectType clz)
        {
            JSClassRelease(clz);
        }
    };
    
    template<>
    class Allocator<JSStringRef>
    {
    public:
        typedef JSStringRef ObjectType;
        static constexpr bool HasContext = false;
        static constexpr bool HasAllocator = false;
        
        static void dealloc(ObjectType obj)
        {
            JSStringRelease(obj);
        }
    };
    
    template<>
    class Allocator<JSValueRef>
    {
    public:
        typedef JSContextRef ContextType;
        typedef JSValueRef ObjectType;
        static constexpr bool HasContext = true;
        static constexpr bool HasAllocator = true;
        
        static void alloc(ContextType ctx, ObjectType obj)
        {
            JSValueProtect(ctx, obj);
        }
        
        static void dealloc(ContextType ctx, ObjectType obj)
        {
            if (ctx)
            {
                JSValueUnprotect(ctx, obj);
            }
        }
    };
    
    template<>
    class Allocator<JSObjectRef> : public Allocator<JSValueRef>
    {
    public:
        typedef JSObjectRef ObjectType;
    };
    
    
    template <typename _Ty>
    class HandlerBase
    {
    public:
        typedef _Ty ObjectType;
        
        HandlerBase()
        {
            m_object = nullptr;
        }
        
        HandlerBase(HandlerBase &&handler)
        {
            (*this) = std::move(handler);
        }
        
        ObjectType Get()const
        {
            return m_object;
        }
        
        void operator=(HandlerBase &&handler)
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
        DISALLOW_COPY(HandlerBase);
    };
    
    template <typename _Ty,
        bool _hasContext = Allocator<_Ty>::HasContext,
        bool _hasAllocator = Allocator<_Ty>::HasAllocator>
    class Handler{};
    
    template <typename _Ty>
    class Handler<_Ty, true, false> : public HandlerBase<_Ty>
    {
    public:
        typedef _Ty ObjectType;
        typedef HandlerBase<_Ty> SuperType;
        typedef Allocator<_Ty> Allocator;
        typedef typename Allocator::ContextType ContextType;
        
        explicit Handler(ContextType ctx, ObjectType obj)
        : m_context(ctx)
        {
            SuperType::m_object = obj;
        }
        virtual ~Handler()
        {
            if (SuperType::m_object)
            {
                Allocator::dealloc(m_context, SuperType::m_object);
            }
        }
        COMMON_MOVE_FUNCTIONS(Handler, SuperType);
        
    protected:
        ContextType m_context;
        
    private:
        DISALLOW_COPY(Handler);
    };
    
    template <typename _Ty>
    class Handler<_Ty, false, true> : public HandlerBase<_Ty>
    {
    public:
        typedef _Ty ObjectType;
        typedef HandlerBase<_Ty> SuperType;
        typedef Allocator<_Ty> Allocator;
        
        Handler()
        {
            SuperType::m_object = Allocator::alloc();
        }
        ~Handler()
        {
            if (SuperType::m_object)
            {
                Allocator::dealloc(SuperType::m_object);
            }
        }
        COMMON_MOVE_FUNCTIONS(Handler, SuperType);
        
    private:
        DISALLOW_COPY(Handler);
    };
    
    template <typename _Ty>
    class Handler<_Ty, false, false> : public HandlerBase<_Ty>
    {
    public:
        typedef _Ty ObjectType;
        typedef HandlerBase<_Ty> SuperType;
        typedef Allocator<_Ty> Allocator;
        
        explicit Handler(ObjectType obj)
        {
            SuperType::m_object = obj;
        }
        ~Handler()
        {
            if (SuperType::m_object)
            {
                Allocator::dealloc(SuperType::m_object);
            }
        }
        COMMON_MOVE_FUNCTIONS(Handler, SuperType);
        
    private:
        DISALLOW_COPY(Handler);
    };
    
    template <typename _Ty>
    class Handler<_Ty, true, true> : public HandlerBase<_Ty>
    {
    public:
        typedef _Ty ObjectType;
        typedef HandlerBase<_Ty> SuperType;
        typedef Allocator<_Ty> Allocator;
        typedef typename Allocator::ContextType ContextType;
        
        explicit Handler(ContextType ctx, ObjectType obj)
        : m_context(ctx)
        {
            SuperType::m_object = obj;
            Allocator::alloc(ctx, obj);
        }
        ~Handler()
        {
            if (SuperType::m_object)
            {
                Allocator::dealloc(m_context, SuperType::m_object);
            }
        }
        COMMON_MOVE_FUNCTIONS(Handler, SuperType);
        
    protected:
        ContextType m_context;
        
    private:
        DISALLOW_COPY(Handler);
    };
}

#endif
