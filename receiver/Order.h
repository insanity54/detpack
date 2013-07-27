
#ifndef Order_h
#define Order_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class Order {
  public:
    Order( const byte orderLen );
    int getType();
    byte gState();
    byte getMark();
    unsigned long getTimeline();
    unsigned long getStartTime();
    byte getNextMark();
    unsigned long getDelayMult();
    
    void incrementMark(byte amt);
    void setState( byte newState );
    void setType( int newType );
    void setDelayMult( unsigned long newDelayMult );
    void setTimeline( unsigned long newTimeline );
    void setStartTime( unsigned long newStartTime );

    
  private:
    byte _orderLen;            // max length of order array
    int _type;                 // holds command payload type
    byte _state;               // parsing state machine current state
    byte _mark;                // mark showing the current parse position
    unsigned long _delayMult;           // delay multiplier for converting delay times to milliseconds
    unsigned long _timeline;   // keeps track of time between actions
    unsigned long _startTime;  // time at which to start carrying out orders
};








#endif //Order.h
