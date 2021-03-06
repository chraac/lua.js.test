//
//  main.cpp
//  lua.test
//
//  Created by hongruichen on 2017/11/27.
//  Copyright © 2017 hongruichen. All rights reserved.
//

#include <iostream>
#include <vector>
#include "luatest.h"

int main(int argc, const char * argv[]) 
{
    std::stringstream ss;
    auto &luatest = LuaTest::TestMain::GetInstance();
    luatest.SetOutputStream(ss);
    luatest.RunAllTest();
    std::cout << ss.str();
    return 0;
}
