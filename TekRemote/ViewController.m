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
    UIImage *buttonBackgroundPressed = [UIImage imageNamed:@"blueButton.png"];
    UIImage *q0ButtonBackgroundPressed = [UIImage imageNamed:@"0blueButton.png"];
    UIImage *q1ButtonBackgroundPressed = [UIImage imageNamed:@"1blueButton.png"];
    UIImage *q2ButtonBackgroundPressed = [UIImage imageNamed:@"2blueButton.png"];
    UIImage *q3ButtonBackgroundPressed = [UIImage imageNamed:@"3blueButton.png"];
    UIImage *q4ButtonBackgroundPressed = [UIImage imageNamed:@"4blueButton.png"];
    [Q0Button setBackgroundImage:q0ButtonBackgroundPressed forState:UIControlStateSelected];
    [Q1Button setBackgroundImage:q1ButtonBackgroundPressed forState:UIControlStateSelected];
    [Q2Button setBackgroundImage:q2ButtonBackgroundPressed forState:UIControlStateSelected];
    [Q3Button setBackgroundImage:q3ButtonBackgroundPressed forState:UIControlStateSelected];
    [Q4Button setBackgroundImage:q4ButtonBackgroundPressed forState:UIControlStateSelected];
    [pst1Button setBackgroundImage:buttonBackgroundPressed forState:UIControlStateSelected];
    [pst2Button setBackgroundImage:buttonBackgroundPressed forState:UIControlStateSelected];
    [pst3Button setBackgroundImage:buttonBackgroundPressed forState:UIControlStateSelected];
    [pst4Button setBackgroundImage:buttonBackgroundPressed forState:UIControlStateSelected];
    [pst5Button setBackgroundImage:buttonBackgroundPressed forState:UIControlStateSelected];
    [pst6Button setBackgroundImage:buttonBackgroundPressed forState:UIControlStateSelected];
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
	[defaults setValue:@"1.2" forKey:@"versionField"];
    NSString *targetIp = [defaults stringForKey:@"ipAddress_preference"];
	session = [[snmpSession alloc]init];
	[session initWithHost:targetIp community:@"private"];
	NSLog(@"Connected with IP %@",targetIp);
}

- (void)clearPresets{
  	[pst1Button setSelected:NO];
	[pst2Button setSelected:NO];
	[pst3Button setSelected:NO];
	[pst4Button setSelected:NO];
	[pst5Button setSelected:NO];
    [pst6Button setSelected:NO];
}

- (void)clearQuardant{
    [Q0Button setSelected:NO];
    [Q1Button setSelected:NO];
    [Q2Button setSelected:NO];
    [Q3Button setSelected:NO];
    [Q4Button setSelected:NO];
  }

- (IBAction)Q1{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:1];
    [self clearQuardant];
    [Q1Button setSelected:YES];
	NSLog(@"Q1");
}

- (IBAction)Q2{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:2];
    [self clearQuardant];
    [Q2Button setSelected:YES];
	NSLog(@"Q2");
}

- (IBAction)Q3{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:3];
    [self clearQuardant];
    [Q3Button setSelected:YES];
	NSLog(@"Q3");
}

- (IBAction)Q4{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:4];
    [self clearQuardant];
    [Q4Button setSelected:YES];
	NSLog(@"Q4");
}

- (IBAction)Q0{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.24.8" value:0];
    [self clearQuardant];
    [Q0Button setSelected:YES];
	NSLog(@"Q0");
}

- (IBAction)PST1{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:1];
    [self clearPresets];
	[pst1Button setSelected:YES];
    [self clearQuardant];
	NSLog(@"PST1");
}

- (IBAction)PST2{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:2];
    [self clearPresets];
	[pst2Button setSelected:YES];
    [self clearQuardant];
	NSLog(@"PST2");
}

- (IBAction)PST3{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:3];
    [self clearPresets];
	[pst3Button setSelected:YES];
    [self clearQuardant];
	NSLog(@"PST3");
}

- (IBAction)PST4{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:4];
    [self clearPresets];
	[pst4Button setSelected:YES];
    [self clearQuardant];
	NSLog(@"PST4");
}

- (IBAction)PST5{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:5];
    [self clearPresets];
    [pst5Button setSelected:YES];
    [self clearQuardant];
	NSLog(@"PST5");
}

- (IBAction)PST6{
	[session setOidIntValue:@"1.3.6.1.4.1.128.5.2.10.12.1" value:6];
    [self clearPresets];
	[pst6Button setSelected:YES];
    [self clearQuardant];
	NSLog(@"PST6");
}

@end
