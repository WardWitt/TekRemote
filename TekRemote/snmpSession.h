//
//  snmpSession.h
//  iPhone_SNMP_Remote
//
//  Created by Ward Witt on 9/20/09.
//  Copyright 2009 Filmworkers Club. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface snmpSession : NSObject {
}

- (id) initWithHost:(NSString*) host community:(NSString*) community;
- (id) stringForOid:(NSString *) objectIdString;
- (void) setOidIntValue:(NSString *)objectIdString value:(int)intVal;

@end
