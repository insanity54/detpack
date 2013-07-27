
#include "Order.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


Order::Order(const byte orderLen) {
  _orderLen = orderLen;
  _type = 0;
  _state = 0;
  _mark = 0;
  _delayMult = 1;
  _timeline = 0;
}


//struct order {
//  int type;              // holds command payload type
//  byte state;            // parsing state machine current state
//  byte mark;             // mark showing the current parse position
//  byte delayMult;        // delay multiplier for converting delay times to milliseconds
//  unsigned long timeline;// keeps track of time between actions 
//};
  
/*
 * Holds command payload type
 */
int Order::getType() {
  
  return _type;
}

byte Order::gState() {
 
   return _state; 
}



byte Order::getMark() {
 
  return _mark; 
}

unsigned long Order::getTimeline() {
 
  return _timeline; 
}

unsigned long Order::getStartTime() {
   return _startTime; 
}

/*
 * Get the number next array element to write orders to.
 * Prevents writing to an out of bounds array element.
 *
 * @return byte           the next element (mark) to use
 */
byte Order::getNextMark() {

  if ( _mark == _orderLen ) {
    return 0;
    
  } else {
    return _mark + 1;
  }  
}

unsigned long Order::getDelayMult() {
  
  return _delayMult;
}

/*
 * request status from the specified receiver.
 * the two character id for this payload is ST
 *
 * @param byte amt  the numeric amount to increase the mark by
 */
void Order::incrementMark( byte amt = 1 ) {
  
  if ( _mark + amt > _orderLen ) {
    // if incrementing by amt will put us over the orderLen limit

    while ( _mark + amt > _orderLen ) {
      // loop until we get a value that is within our limit
      
      // find the amount over the limit it will make us
      // store that amount
      amt = ( _mark + amt ) - _orderLen;
      
      // blah blah math
      _mark = amt;
    }
      

  } else {
    _mark += amt;
  }
}

void Order::setState( byte newState ) {
  _state = newState;
}

void Order::setType( int newType ) {
  _type = newType;
}

void Order::setDelayMult( unsigned long newDelayMult ) {
  _delayMult = newDelayMult; 
}

void Order::setTimeline( unsigned long newTimeline ) {
  _timeline = newTimeline;
}

void Order::setStartTime( unsigned long newStartTime ) {
  _startTime = newStartTime;
}
