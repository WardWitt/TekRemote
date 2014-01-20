//
//  ViewController.m
//  TekRemote
//
//  Created by Ward Witt on 1/10/14.
//  Copyright (c) 2014 Ward Witt. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [self openSNMPConnection];
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)openSNMPConnection {
    // load up deafults with something sane for first run
    NSDictionary *IPDefault = [NSDictionary dictionaryWithObject:@"0.0.0.0" forKey:@"ipAddress_preference"];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults registerDefaults:IPDefault];
	[defaults setValue:@"1.1" forKey:@"versionField"];
    NSString *targetIp = [defaults stringForKey:@"ipAddress_preference"];
	session = [[snmpSession alloc]init];
	[session initWithHost:targetIp community:@"private"];
	NSLog(@"Connected with IP %@",targetIp);
}

- (IBAction)Q1{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:1];
	NSLog(@"Q1");
}

- (IBAction)Q2{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:2];
	NSLog(@"Q2");
}

- (IBAction)Q3{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:3];
	NSLog(@"Q3");
}

- (IBAction)Q4{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:4];
	NSLog(@"Q4");
}

- (IBAction)Q0{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:0];
	NSLog(@"Q0");
}

- (IBAction)PST1{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:1];
	[pst1Button setSelected:YES];
	[pst2Button setSelected:NO];
	[pst3Button setSelected:NO];
	[pst4Button setSelected:NO];
	[pst5Button setSelected:NO];
	NSLog(@"PST1");
}

- (IBAction)PST2{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:2];
	[pst1Button setSelected:NO];
	[pst2Button setSelected:YES];
	[pst3Button setSelected:NO];
	[pst4Button setSelected:NO];
	[pst5Button setSelected:NO];
	NSLog(@"PST2");
}

- (IBAction)PST3{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:3];
	[pst1Button setSelected:NO];
	[pst2Button setSelected:NO];
	[pst3Button setSelected:YES];
	[pst4Button setSelected:NO];
	[pst5Button setSelected:NO];
	NSLog(@"PST3");
}

- (IBAction)PST4{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:4];
	[pst1Button setSelected:NO];
	[pst2Button setSelected:NO];
	[pst3Button setSelected:NO];
	[pst4Button setSelected:YES];
	[pst5Button setSelected:NO];
	NSLog(@"PST4");
}

- (IBAction)PST5{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:5];
	[pst1Button setSelected:NO];
	[pst2Button setSelected:NO];
	[pst3Button setSelected:NO];
	[pst4Button setSelected:NO];
	[pst5Button setSelected:YES];
	NSLog(@"PST5");
}

@end
