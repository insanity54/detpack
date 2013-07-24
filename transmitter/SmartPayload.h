/*
  SmartPayload.h - Library for sending data to DetPack receivers
  Created by Chris Grimmett of Grimtech, LLC.
  xtoast@gmail.com
  www.grimtech.net
  
  Public Domain, no warranty, attribution appreciated.
*/



#ifndef SmartPayload_h
#define SmartPayload_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class SmartPayload {
  public:
    SmartPayload();
    byte requestIgnite(byte charge, boolean action, byte pos);
    void requestStatus();
    void requestScript(byte script);
    void requestStrafe();
    void requestCounterStrike();
    void requestC4();
  private:
    byte _receiver;
    byte _charge;
};








#endif //SmartPayload.h
