# Copyright (c) 2017 The Tencent Authors. All rights reserved.

cmake_minimum_required(VERSION 3.4.3)

project(lua.test C CXX)

include(systemchecker.cmake)

file(GLOB lua_srcs
    "./lua/*.c*"
)
add_library(lua STATIC
    ${lua_srcs}
)
target_include_directories(lua PUBLIC
    "./lua"	
)
target_compile_definitions(lua PUBLIC
    LUA_LIB
    LUA_COMPAT_PACKAGELIB
    LUA_COMPAT_IOLIB
    luai_ctype_=luai_ctype__
    luaO_nilobject_=luaO_nilobject__
    luaP_opnames=luaP_opnames_
    luaP_opmodes=luaP_opmodes_
)
target_compile_options(lua PUBLIC 
    -fexceptions
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
)
set_target_properties(lua PROPERTIES 
    XCODE_ATTRIBUTE_GCC_INPUT_FILETYPE "sourcecode.cpp.cpp"
    XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "YES"
)

if(IS_WINDOWS)
    add_executable(lua.test
        "main.cpp"
    )
    set_source_files_properties(${lua_srcs} PROPERTIES LANGUAGE CXX)
elseif(IS_MAC OR IS_IOS)
    add_executable(lua.test MACOSX_BUNDLE
        "./luatest.h"
        "./jstest.h"
        "./testcommon.h"
        "./handler.h"
        "./ios/main.m"
        "./ios/AppDelegate.m"
        "./ios/ViewController.mm"
        "./ios/Main.storyboard"
        "./ios/Info.plist"
    )
    set(SDK_ROOT ${CMAKE_IOS_SDK_ROOT})
endif()

target_compile_options(lua.test PRIVATE 
    -fexceptions
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
)

if(IS_MAC OR IS_IOS)
    target_include_directories(lua.test PUBLIC
        "./"
    )
    find_library(UIKIT UIKit)
    find_library(FOUNDATION Foundation)
    find_library(JSCORE JavaScriptCore)
    target_link_libraries(lua.test PRIVATE
        lua
        ${FOUNDATION}
        ${UIKIT}
        ${JSCORE}
    )
    set_target_properties(lua.test PROPERTIES
        RESOURCE "./ios/Main.storyboard"
        XCODE_ATTRIBUTE_ALWAYS_SEARCH_USER_PATHS "NO"
        XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
        XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "YES"
        XCODE_ATTRIBUTE_GCC_DYNAMIC_NO_PIC "0"
        XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "NO"
        XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "YES"
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
        XCODE_ATTRIBUTE_COPY_PHASE_STRIP "YES"
        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
        XCODE_ATTRIBUTE_ENABLE_STRICT_OBJC_MSGSEND "YES"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "7.0"
        XCODE_ATTRIBUTE_SDKROOT "iphoneos"
        XCODE_ATTRIBUTE_VALID_ARCHS "arm64 armv7 armv7s i386 x86_64"
        XCODE_ATTRIBUTE_OTHER_LDFLAGS "-ObjC"
        XCODE_ATTRIBUTE_SKIP_INSTALL "YES"
        XCODE_ATTRIBUTE_INFOPLIST_FILE "$(SRCROOT)/ios/Info.plist"
        XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
        XCODE_ATTRIBUTE_MACH_O_TYPE "mh_execute"
    )
endif()