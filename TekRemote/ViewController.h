//
//  ViewController.h
//  TekRemote
//
//  Created by Ward Witt on 1/10/14.
//  Copyright (c) 2014 Ward Witt. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "snmpSession.h"

@interface ViewController : UIViewController {
    
    snmpSession * session;
    IBOutlet UIButton * Q0Button;
    IBOutlet UIButton * Q1Button;
    IBOutlet UIButton * Q2Button;
    IBOutlet UIButton * Q3Button;
    IBOutlet UIButton * Q4Button;
    IBOutlet UIButton * pst1Button;
    IBOutlet UIButton * pst2Button;
    IBOutlet UIButton * pst3Button;
    IBOutlet UIButton * pst4Button;
    IBOutlet UIButton * pst5Button;
    IBOutlet UIButton * pst6Button;
}

- (void)openSNMPConnection;
- (IBAction)PST1;
- (IBAction)PST2;
- (IBAction)PST3;
- (IBAction)PST4;
- (IBAction)PST5;

- (IBAction)Q1;
- (IBAction)Q2;
- (IBAction)Q3;
- (IBAction)Q4;
- (IBAction)Q0;

@end
