# Copyright (c) 2017 The Tencent Authors. All rights reserved.

cmake_minimum_required(VERSION 3.4.3)

project(lua.test C CXX)

include(systemchecker.cmake)

if(IS_WINDOWS)
    set(EXE_TYPE "")
elseif(IS_MAC)
    set(EXE_TYPE MACOSX_BUNDLE)
endif()

add_executable(lua.test ${EXE_TYPE}
    "main.cpp"
)

file(GLOB lua_srcs
    "./lua/*.c*"
)
target_sources(lua.test PRIVATE
    ${lua_srcs}
)
set_source_files_properties(${lua_srcs} PROPERTIES LANGUAGE CXX)
target_include_directories(lua.test PUBLIC
    "./lua"	
)
target_compile_definitions(lua.test PRIVATE 
    LUA_LIB
)
target_compile_options(lua.test PRIVATE 
    -fexceptions
    -std=c++11
)

if(IS_MAC)
    target_compile_definitions(lua.test PRIVATE 
        LUA_COMPAT_PACKAGELIB
        LUA_COMPAT_IOLIB
        luai_ctype_=luai_ctype__
        luaO_nilobject_=luaO_nilobject__
        luaP_opnames=luaP_opnames_
        luaP_opmodes=luaP_opmodes_
    )
    set_target_properties(lua.test PROPERTIES 
        XCODE_ATTRIBUTE_GCC_INPUT_FILETYPE "sourcecode.cpp.cpp"
        XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "YES"
    )
endif()