//
//  testcommon.h
//
//  Created by hongruichen on 2017/11/27.
//  Copyright © 2017 hongruichen. All rights reserved.
//

#ifndef __TESTCOMMON_H__
#define __TESTCOMMON_H__

#define DISALLOW_COPY(clz) clz(clz&)=delete;void operator=(clz&)=delete
#define DISALLOW_MOVE(clz) clz(clz&&)=delete;void operator=(clz&&)=delete

#include <sstream>
#include <functional>

namespace TestCommon
{
    template<typename _Ty>
    class TestBase
    {
    public:
        typedef std::function<void()> TestEvent;
        
        static _Ty &GetInstance()
        {
            static _Ty instance;
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
        
        
        virtual void RunAllTest(TestEvent start, TestEvent end, size_t count) = 0;
        
    protected:
        std::stringstream *m_ss;
        
        TestBase() { m_ss = nullptr; }
        
    private:
        DISALLOW_COPY(TestBase);
        DISALLOW_MOVE(TestBase);
    };
}

#endif
