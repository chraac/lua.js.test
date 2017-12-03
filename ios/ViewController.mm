//
//  ViewController.m
//  lua.test
//
//  Created by hongruichen on 2017/12/1.
//  Copyright © 2017年 hongruichen. All rights reserved.
//

#import "ViewController.h"
#include "luatest.h"
#include "jstest.h"
#include <iostream>
#import <mach/mach.h>

namespace
{
    vm_size_t GetMemSize()
    {
        task_basic_info info = {0};
        mach_msg_type_number_t size = sizeof(info);
        kern_return_t err = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
        return err == KERN_SUCCESS? info.resident_size: 0;
    }
    
    vm_size_t GetVirtualSize()
    {
        task_basic_info info = {0};
        mach_msg_type_number_t size = sizeof(info);
        kern_return_t err = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
        return err == KERN_SUCCESS? info.virtual_size: 0;
    }
}

@interface ViewController ()

@end

@implementation ViewController {
    __weak IBOutlet UITextView *_textView;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}

- (IBAction)luaTestClick:(id)sender {
    NSLog(@"luaTestClick");
    std::stringstream ss;
    ss.str().reserve(4096);
    auto &test = LuaTest::TestMain::GetInstance();
    test.SetOutputStream(ss);
    
    vm_size_t startSize;
    vm_size_t startVirtualSize;
    vm_size_t endSize;
    vm_size_t endVirtualSize;
    
    test.RunAllTest([&]()
    {
        startSize = GetMemSize();
        startVirtualSize = GetVirtualSize();
    }, [&]()
    {
        endSize = GetMemSize();
        endVirtualSize = GetVirtualSize();
    });
    
    ss << "Memory.Physical.Size(bytes):" << endSize - startSize << std::endl;
    ss << "Memory.Virtual.Size(bytes):" << endVirtualSize - startVirtualSize << std::endl;
    
    [_textView setText:[NSString stringWithUTF8String:ss.str().c_str()]];
}
- (IBAction)jsTestClick:(id)sender {
    NSLog(@"jsTestClick");
    std::stringstream ss;
    ss.str().reserve(4096);
    auto &test = JSTest::TestMain::GetInstance();
    test.SetOutputStream(ss);
    
    vm_size_t startSize;
    vm_size_t startVirtualSize;
    vm_size_t endSize;
    vm_size_t endVirtualSize;
    
    test.RunAllTest([&]()
    {
        startSize = GetMemSize();
        startVirtualSize = GetVirtualSize();
    }, [&]()
    {
        endSize = GetMemSize();
        endVirtualSize = GetVirtualSize();
    });
    
    ss << "Memory.Physical.Size(bytes):" << endSize - startSize << std::endl;
    ss << "Memory.Virtual.Size(bytes):" << endVirtualSize - startVirtualSize << std::endl;
    
    [_textView setText:[NSString stringWithUTF8String:ss.str().c_str()]];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
