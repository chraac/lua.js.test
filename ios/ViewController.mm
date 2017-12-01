//
//  ViewController.m
//  lua.test
//
//  Created by hongruichen on 2017/12/1.
//  Copyright © 2017年 hongruichen. All rights reserved.
//

#import "ViewController.h"

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
}
- (IBAction)jsTestClick:(id)sender {
    NSLog(@"jsTestClick");
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
