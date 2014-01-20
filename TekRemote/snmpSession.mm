//
//  snmpSession.mm
//  iPhone_SNMP_Remote
//
//  Created by Ward Witt on 9/20/09.
//  Copyright 2009 Filmworkers Club. All rights reserved.
//

#import "snmpSession.h"
#import "snmp_pp.h"

@implementation snmpSession
int status;
NSString *gHost;
NSString *gCommunity;

- (id) initWithHost:(NSString*) host community:(NSString*) community
{
	gHost = [NSString stringWithString:host];
	gCommunity = [NSString stringWithString:community];
	return self;
}

- (id) stringForOid:(NSString *) objectIdString
{
	id result = nil;
	Snmp::socket_startup();  //Init socket subsystem
	UdpAddress address([gHost UTF8String]); //10.0.0.126 //evertz-frame-2.filmworkers.com
	Oid oid([objectIdString UTF8String]);      // RTC
	
	snmp_version version=version1;                  // default is v1
	int retries=1;                                  // default retries is 1
	int timeout=100;                                // default is 1 second
	u_short port=161;                               // default snmp port is 161
	OctetStr community([gCommunity UTF8String]);    // community name
	
	//----------[ create a SNMP++ session ]-----------------------------------
	int status;
	// bind to any port and use IPv6 if needed
	Snmp snmp(status, 0, (address.get_ip_version() == Address::version_ipv6));
	
	if ( status != SNMP_CLASS_SUCCESS) {
		NSLog(@"SNMP++ Session Create Fail");
	}
	//--------[ build up SNMP++ object needed ]-------------------------------
	Pdu pdu;                               // construct a Pdu object
	Vb vb;                                 // construct a Vb object
	vb.set_oid( oid);                      // set the Oid portion of the Vb
	pdu += vb;                             // add the vb to the Pdu
	
	address.set_port(port);
	CTarget ctarget( address);             // make a target using the address
	ctarget.set_version( version);         // set the SNMP version SNMPV1 or V2
	ctarget.set_retry( retries);           // set the number of auto retries
	ctarget.set_timeout( timeout);         // set timeout
	ctarget.set_readcommunity( community); // set the read community name
	
	SnmpTarget *target;
	target = &ctarget;
	
	status = snmp.get( pdu, *target);
	
	if (status == SNMP_CLASS_SUCCESS)
	{
		pdu.get_vb( vb,0);
		switch (vb.get_syntax()) {
			case 2: // Integer
				int i;
				vb.get_value(i);
				result = [NSString stringWithFormat:@"%i",i];
				Snmp::socket_cleanup();
				return result;
				break;
			case 4: // String
				char buf[100];
				vb.get_value(buf);
				result = [NSString stringWithFormat:@"%s", buf];
				Snmp::socket_cleanup();
				return result;
				break;
			default:
				NSLog(@"Unsupported data type 0x02x for \"%lu\"",vb.get_syntax());
				Snmp::socket_cleanup();
				break;
		}
		
		
	}
	
	NSString * SNMPTimeout = @"";
	NSLog(@"SNMP failed");
	Snmp::socket_cleanup();
	return SNMPTimeout;
	
}

- (void) setOidIntValue:(NSString *)objectIdString value:(int)intVal
{
	Snmp::socket_startup();  //Init socket subsystem
	UdpAddress address([gHost UTF8String]); //10.0.0.126 //evertz-frame-2.filmworkers.com
	Oid oid([objectIdString UTF8String]);      // RTC
	
	snmp_version version=version1;                  // default is v1
	int retries=1;                                  // default retries is 1
	int timeout=100;                                // default is 1 second
	u_short port=161;                               // default snmp port is 161
	OctetStr community([gCommunity UTF8String]);    // community name
	
	//----------[ create a SNMP++ session ]-----------------------------------
	int status;
	// bind to any port and use IPv6 if needed
	Snmp snmp(status, 0, (address.get_ip_version() == Address::version_ipv6));
	
	if ( status != SNMP_CLASS_SUCCESS) {
		NSLog(@"SNMP++ Session Create Fail");
	}
	//--------[ build up SNMP++ object needed ]-------------------------------
	Pdu pdu;                               // construct a Pdu object
	Vb vb;                                 // construct a Vb object
	vb.set_oid( oid);                      // set the Oid portion of the Vb
	vb.set_value(intVal);			       // set int value
	pdu += vb;                             // add the vb to the Pdu
	
	address.set_port(port);
	CTarget ctarget( address);             // make a target using the address
	ctarget.set_version( version);         // set the SNMP version SNMPV1 or V2
	ctarget.set_retry( retries);           // set the number of auto retries
	ctarget.set_timeout( timeout);         // set timeout
	ctarget.set_readcommunity( community); // set the read community name
	ctarget.set_writecommunity(community);
	
	SnmpTarget *target;
	target = &ctarget;
	
	status = snmp.set( pdu, *target);
	Snmp::socket_cleanup();
}


@end
