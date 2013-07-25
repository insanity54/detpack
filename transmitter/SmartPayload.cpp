/*
  SmartPayload.cpp - Library for sending data to DetPack receivers
  Created by Chris Grimmett of Grimtech, LLC.
  xtoast@gmail.com
  www.grimtech.net
  
  Public Domain, no warranty, attribution appreciated.
*/

#include "SmartPayload.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


SmartPayload::SmartPayload() {
  
}

/*
 * send command to ignite the specified receiver charge.
 * the two character id for this payload is IG
 *
 * @param charge byte            the charge number. zero index.
 * @return byte array pointer    the command to send over the air
 */
byte SmartPayload::requestIgnite(byte charge, boolean action, byte pos) {
  // assemble smart payload
  // needs to be a byte array
  // full command format is IGrq
  // where IG is 'ignite', r is remote number, and q is charge number
  
  // command ID (IG for ignite)
  byte payload[] = { 73, 71, charge, action}; // 73 = 'I', 71 = 'G'
  
  
  
  
  // command ID (IG for ignite)
  //unsigned long payload = 7371;  // 73 = 'I', 71 = 'G'
  
  // append receiver number and charge number
  //payload = payload * 100 + receiver;
  //payload = payload * 100 + charge;
  
  
  
  
  
    
  // return the command
  return payload[pos];
  
  
  
}


/*
 * request status from the specified receiver.
 * the two character id for this payload is ST
 *
 * @param receiver
 */
void SmartPayload::requestStatus() {
  // send command to request receiver status 
}
  
void SmartPayload::requestScript(byte script) {
   // send specified firing script to receivers 
}

void SmartPayload::requestStrafe() {
  // @todo figure out parameters needed for this function
}

void SmartPayload::requestCounterStrike() {
  // @todo figure out params needed for this function 
}

void SmartPayload::requestC4() {
  // @todo figure out params needed for this function
}
