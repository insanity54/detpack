
// DESCRIPTION
/*
 @todo add to the description
 @todo make a fritzing chart

   * SoftwareSerial RX digital pin 2
   * SoftwareSerial TX digital pin 3
 
   * Speaker on pin 12
   * LED on pin 13
   
   * Charge 1 on pin 4
   * Charge 2 on pin 5
   * Charge 3 on pin 6
   * Charge 4 on pin 7
   * Charge 5 on pin 8
   * Charge 6 on pin 9
   * Charge 7 on pin 10
   * Charge 8 on pin 11
   * Charge 9 on pin 14
   * Charge 10 on pin 15
   * Charge 11 on pin 16
   * Charge 12 on pin 17
   * Charge 13 on pin 18
   * Charge 14 on pin 19
 
   

 Conventions:
   * variable names are lowercase with underscores
   * no abbreviations in variables
   
   * functions should be should be featureFunctionSubfunction
     * Examples
       * displayUpdateServo
       * displayUpdateSerial
       * displayClearServo
       * displayClearTime
   
   * Timers should be timerFeatureFunction
     * Examples
       * timerServoTest
       * timerDisplayServo
       
   * Numbers used in code start at zero, so Remote 1 AFK is Remote 0 in code.
     They are converted to human readable numbers (R0 is now R1) right before being displayed.
     * It's only Remote 1 to the user. It's actually Remote 0!!!

 */


// INCLUDE
#include <XBee.h>
#include <SoftwareSerial.h>
#include <SimpleTimer.h>
#include "Order.h"


// PIN CONNECTIONS
uint8_t ssRx = 11;                         // software serial RX
uint8_t ssTx = 12;                         // software serial TX
uint8_t led = 13;                          // status LED
uint8_t chargePin[] = { 2, 3, 4, 5, 6, 7}; // charges

// CONFIG
byte me = 1; // zero-index of this receiver's number. @todo make this automatic. see below or search for, "automatic receiver id"
//boolean debugMode = 0;
unsigned int ignitionHold = 4000;           // pause time after igntion before DIO pin goes back LOW
const byte commandLen = 48;  // length of command payload.
const byte orderLen = 24;    // length of locally stored order lists

// INIT
byte shCmd[] = {'S', 'H'};  // serial high
byte slCmd[] = {'S', 'L'};  // serial low
SoftwareSerial nss(ssRx, ssTx);
XBee xbee = XBee();
AtCommandRequest atRequest = AtCommandRequest(shCmd);
AtCommandResponse atResponse = AtCommandResponse();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
//SimpleTimer timer;
Order order1 = Order(orderLen);


//struct order {
//  int type;              // holds command payload type
//  byte state;            // parsing state machine current state
//  byte mark;             // mark showing the current parse position
//  byte delayMult;        // delay multiplier for converting delay times to milliseconds
//  unsigned long timeline;// keeps track of time between actions 
//};
  
//order order0 = {0, 0, 0, 0, 0};

int actionList[orderLen][2] = {
  {0, 1},  // charge number, activate (1) or de-activate (0)
  {1, 1},
  {2, 1},
  {3, 1},
  {4, 1},
  {5, 1},
  {0, 0},
  {1, 0},
  {2, 0},
  {3, 0},
  {4, 0},
  {5, 0},
  {0, 1},
  {1, 1},
  {2, 1},
  {3, 1},
  {4, 1},
  {5, 1},
  {0, 0}, 
  {1, 0},
  {2, 0},
  {3, 0},
  {4, 0},
  {5, 0},
};

long actionTime[orderLen] = {
  1000,    // time in ms
  1000,
  2000,
  3000,
  4000,
  5000,
  6000,
  7000,
  8000,
  9000,
  10000,
  11000,
  12000,
  13000,
  14000,
  15000,
  16000,
  17000,
  18000,
  19000,
  20000,
  20000,
  20000,
  20000,
};

boolean actionEnable[orderLen] = {0}; // = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

//byte supportedCommands[2][2] = {
//  {'I', 'G'}, // Ignite. Ignites the specified receiver immediately
//  {'F', 'S'}, // Fire Script. Accepts firing orders and waits for GO
//  {'D', 'A'}  // De-activate. Turns all outputs OFF
//};


void setup() {
  nss.begin(9600);
  nss.println("setting up");

  digitalWrite(led, HIGH);
  Serial.begin(9600);
  xbee.begin(Serial);

  // set charge pins as outputs
  for ( int c = 0; c < sizeof(chargePin); c++ ) {
    pinMode(chargePin[c], OUTPUT);
    //digitalWrite(chargePin[c], OUTPUT); // test light. @todo deleteme
  }
  
  // find receiver's address
  // @todo implement automatic receiver id
  //   - receiver powers on
  //   - receiver asks transmitter for it's receiver number
  //   - transmitter replies with receiver number
  //int address = myAddress();
  //nss.print("my address:");
  //nss.println(me);
  
  // keep track of time
  //unsigned long startTime = millis();
}

void loop() {
  // check xbee for commands
  xbeeCheck();
  //timer.run();
  processOrder();
  //testAction();
}

/*
 * Continually check xbee for commands from the transmitter
 */
void xbeeCheck(){
  // Continuously read packets
  xbee.readPacket();
  
  if (xbee.getResponse().isAvailable()) {
    // got something
    nss.println("got something");
    
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      // got a zb rx packet
      nss.println("got a zb rx packet");
      
      // fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);
      
      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
        // the sender got an ACK
        nss.println("The sender got an ACK");
        
      } else {
        // we got the packet but sender didn't get an ACK
        nss.println("We got the packet but the sender didn't get an ACK");
      }
      
      // display the data we got
      nss.print("Data length is: ");
      nss.println(rx.getDataLength());
      
      nss.print("The data we got: ");
      for (int character = 0; character < rx.getDataLength(); character++) {
        
        nss.print(rx.getData(character));
        nss.print(",");
      }
      nss.println();
      
      // parse that data!
      smartParse();

      
    } else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
      // the local xbee sends this modem status response on certain events, like association/dissasociation
      xbee.getResponse().getModemStatusResponse(msr);
      
      if (msr.getStatus() == ASSOCIATED) {
        // the modem is associated
        nss.println("the modem is associated!");
        
      } else if (msr.getStatus() == DISASSOCIATED) {
        // modem is disassociated
        nss.println("The modem is disassociated!");
        
      } else {
        // some other status
        nss.println("The modem is ????? (unknown status type)");
      }
      
    } else {
      // not something we were expecting
      nss.println("Something unexpected happened. We received an unsupported API frame.");
      
      nss.print("We got API frame: ");
      nss.println(xbee.getResponse().getApiId(), HEX);  
  }

  } else if (xbee.getResponse().isError()) {
    nss.print("xbee packet error. Error code: ");
    nss.println(xbee.getResponse().getErrorCode());
  }    
}







// ccc
void processOrder() {
  // iterate through rows of the action lists
  for ( int row = 0; row < orderLen; row ++ ) {
    // @todo a possible performance optimization here is that we don't go
    // to the next row until the previous row has been executed.
    
    if ( actionEnable[ row ] ) {
      // if action is enabled on this row
      
      if ( millis() - order1.getStartTime() >= actionTime[ row ] ) {
        // if it is time to carry out the order on this row
      
        // ignite or de-activate charge depending on what the list is telling us to do
        setCharge( actionList[ row ][ 0 ], actionList[ row ][ 1 ] );
        nss.print( "culprit:" );
        nss.println( row );
        
        // disable action so we don't repeat
        actionEnable[ row ] = 0;
        
      } 
    }
  }
}

//void testAction() {
//  
//  // iterate through rows of the lists
//  for ( int row = 0; row < 24; row ++ ) {  
//    
//    //nss.print("milli");
//    //nss.print(millis());
//    
//    //nss.print(" | iter");
//    //nss.print(row);
//    
//    //nss.print(" | act");
//    //nss.print(act[row]);
//      
//    if ( actionEnable[row] ) {
//      // if we need to act
//    
//      // @todo we need a startTime reference point from the time that parsing completed.
//      if ( millis() >= actionTime[row] ) {
//        // it's time to take action
//        
//        setCharge(actionList[row][0], actionList[row][1]);
//        
//        // we've acted, so don't act next time
//        actionEnable[row] = 0;
//      }
//    }
//    //nss.println();
//  }
//}





//unsigned int sendAtCommand() {
//  unsigned int response = 0;
//  
//  if (xbee.readPacket(5000)) {
//    // got response
//    
//    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
//      xbee.getResponse().getAtCommandResponse(atResponse);
//      
//      if (atResponse.isOk()) {
//        nss.println("command ok");
//        
//        if (atResponse.getValueLength() > 0) {
//          // got nice fresh & tasty response right here
//                    
//          return response;
//          
//        }
//      }
//    }
//  }
//}


///*
// * Returns address of connected xbee
// */
//unsigned int myAddress() {
//  unsigned int sh = 0;
//  unsigned int sl = 0;
//  unsigned int me[] = {sh, sl};
//  
//  sh = sendAtCommand();
//
//  atRequest.setCommand(slCmd);
//  
//  sl = sendAtCommand();
//  
//  
//  
//  return me[0];
//}

///*
// * Parses the data from the transmitter
// * validates the data
// * and processes it accordingly.
// */
//void parseData() {
//  // find the command id
//  // exec the first set
//  // find the second set
//  // find the third set
//  
//  // find command id
//  int commandId;
//  commandId = rx.getData(0);
//  commandId = commandId * 100 + rx.getData(1); // concatenate data0 with data1
//  nss.print("commandId:");
//  nss.println(commandId);
//  
//  // find first set
//  byte pak[2] = {0};
//  pak[0] = rx.getData(2);
//  pak[1] = rx.getData(3);
//  
//  switch (commandId) {
//  case 6865:
//    // 'DA' De-activate command
//    nss.println("de-activating");
//    for (byte charge = 0; charge <= sizeof(chargePin); charge++) {
//      setCharge(charge, 0);
//    }
//    break;    
//    
//  case 7083:
//    // 'FS' Fire Script command
//    nss.print(">> FS:");
//    nss.print(pak[0]);
//    nss.print(",");
//    nss.println(pak[1]);
//    
//    if (pak[0] == me) {
//      setCharge(pak[1], 1); // set the charge number HIGH
//      nss.print("ignite:");
//      nss.println(pak[1]);
//    }
//    break;
//    
//  default:
//    // Unsupported command
//    nss.print("unsuportd cmd");
//
//    // @todo add more modes. Fun!
//  }
//}

void setCharge(byte charge, bool setStatus) {
  nss.print("setting charge: ");
  nss.print(charge);
  if (setStatus) {
    nss.println(" HIGH");
  } else {
    nss.println(" LOW");
  }

  if (setStatus) {
    // activating (igniting)
    digitalWrite(charge[chargePin], HIGH);
    
    // set timer to de-activate charge after hold time has expired
    //int deactivateTimer = timer.setTimeout(ignitionHold, deactivateCharges);

  } else {
    // de-activating
    digitalWrite(charge[chargePin], LOW);
  }
}





/*
 * smartParse takes the received command payload and goes through it character
 * by character and determines what it is that needs to be done with the
 * orders. smartParse will discard orders in the command which belong to receivers
 * other than itself.
 *
 * From the command payload, smartParse comes up with two lists (arrays) containing:
 * 1) actions
 * 2) times
 *
 * The action list dictates the charge number and either activate or de-activate.
 * The time list dictates the relative time at which to take action.
 *
 * smartParse accepts no parameters. Instead, the information it needs is gathered
 * from the global 'command' array.
 */
void smartParse() {
  // State machine
  // looking for: 0 = first char of payload type
  //              1 = second char of payload type
  //              2 = receiver number or special command
  //              3 = charge number
  //              4 = special command parameter
  //              5 = ignore this value
  
  // reset some jazz
  order1.setTimeline( 0 );
  
  // @todo this should allow multiple orders, ie: first order comes in and
  //       it's made to be order0. second order comes in, it's order1.
  
  // while there are chars to parse
  byte charPos = 0;
  order1.setState( 0 );
  while ( charPos < commandLen ) {
    int inspectChar = rx.getData( charPos );
    
    
    
    switch ( order1.gState() ) {

      
    /*
     * Parsing State 0
     * looking for first char of payload type
     */
    case 0:
      nss.println("|| state0");
      order1.setType( inspectChar );
      order1.setState( 1 ); // go to next state
      break;
    
    
    /*
     * Parsing State 1
     * looking for the second char of payload type
     */    
    case 1:
      nss.println("|| state1");
      // concatenate first and second char of payload type and store in order struct
      order1.setType( order1.getType() * 100 + inspectChar );
      
      // determine payload type
      switch ( order1.getType() ) {
      case 6865:
        // DA - De-Activate
        nss.println(">> DA");
        // @todo activate DA somehow
        break;
          
      case 7083:
        // FS - Fire Script
        nss.println(">> FS");
        // @todo activate FS somehow
        break;
        
      // @todo add more payload types. Fun!
        
      default:
        // unrecognized payload type.
        nss.println("unsupp panic");
        charPos = commandLen;  // End parsing immediately.
        
        
      } // end of switch
      
      order1.setState( 2 ); // go to next state
      break;
      
      
    /*
     * Parsing State 2
     * looking for receiver number or special command
     */        
    case 2:
      nss.println("|| state2");
      
      if ( inspectChar < 100 ) {
        // character is less than 100 so it's got to be a receiver number,
        // not a special command (special commands are numbers above 100)
        
        if ( inspectChar == me ) {
          // if character is my receiver number
          
          // move to next state in which we will treat the next
          // character as a charge number
          order1.setState( 3 );

          
        } else {
          // character is less than 100 but it's not my number.
          // we need to ignore this and the next character.
          order1.setState( 5 );
        }
        
      } else {
        // character is greater than 100 so it's got to be a
        // special command.
        
        switch ( inspectChar ) {
        // find what special command has been issued

        case 128:
          // end of line. stop parsing.
          charPos = commandLen;
          break;
          
        case 120:
          // delay time will be mlliseconds
          
          order1.setDelayMult( 1 );
          order1.setState( 4 );
          break;
          
        case 121:
          // delay time will be seconds
          order1.setDelayMult( 1000 );
          order1.setState( 4 );
          nss.print("delaymult:");  // @todo delete this debug
          nss.println(order1.getDelayMult());
          break;
          
        case 122:
          // delay time will be minutes
          order1.setDelayMult( 60000 );
          order1.setState( 4 );
          break;
          
        default:
          // unrecognized/unsupported special command type
          
          nss.println("unsupp 100 cmd");
          // We don't understand the command but a different type or version receiver might.
          // We will ignore this command and the next character, the command parameter.
          order1.setState( 5 );
          
          
        } // end of switch
      }   // end of else
      
      break;
  
      
    /*
     * Parsing State 3
     * looking for charge number
     */      
    case 3:
      nss.println( "|| state3" );
      
      // add this charge number to our list of orders
      actionList[ order1.getMark() ][ 0 ] = inspectChar;
      
      // indicate that we want to ignite
      actionList[ order1.getMark() ][ 1 ] = 1;
      
      // add the time at which our action needs to take place
      actionTime[ order1.getMark() ] = order1.getTimeline();
      
      // enable the action
      actionEnable[ order1.getMark() ] = 1;
      
      // set a de-activate command x milliseconds in the future
      // where x is the value of the globally configured ignition hold. (ignitionHold)
      actionList[ order1.getNextMark() ][0] = inspectChar;                // array row
      actionList[ order1.getNextMark() ][1] = 0;                          // LOW
      actionTime[ order1.getNextMark() ] = order1.getTimeline() + ignitionHold;// time at which to go LOW
      actionEnable[ order1.getNextMark() ] = 1;                           // enable
      
      // debug info
      nss.print( "(t):" );
      nss.println( order1.getTimeline() );
      
      // increment mark by 2 so we don't overwrite these orders next iteration
      order1.incrementMark( 2 );
          
      // find out what our next character is
      order1.setState( 2 );
      break;

      
    /*
     * Parsing State 4
     * looking for special command parameter
     */        
    case 4:
      nss.println("|| state4");
      
      nss.print( "** pre-tl:" );
      nss.print( order1.getTimeline() );
      nss.print( " inspect:" );
      nss.print( inspectChar );
      nss.print( " mult:");
      nss.print( order1.getDelayMult() );
      nss.print( " mark:");
      nss.println( order1.getMark() );
      
      
      { 
        

        // convert the time to milliseconds depending on the time unit (millis, seconds, minutes)
        // that we gathered from state 2. Store in our actionTime list
        unsigned long delayTime = order1.getTimeline() + inspectChar * order1.getDelayMult();
        actionTime[ order1.getMark() ] = delayTime;
        order1.setTimeline( delayTime );  // running tab of order timeline
        // ccc
      
      nss.print( "** post-tl:" );
      nss.print( order1.getTimeline() );
      nss.print( " actionTime:" );  
      nss.print( actionTime[ order1.getMark() ] );
      nss.print( " delaytime:" );
      nss.print( delayTime );
      nss.print( " mult:");
      nss.println( order1.getDelayMult() );
      
      }
      // find out what our next character is
      order1.setState(2);
      break;

      
    /*
     * Parsing State 5
     * looking for nothing; we are ignoring this value.
     * we are ignoring because the value is not meant for us.
     * Most likely we picked up this value from a transmitter broadcast
     * and the value is a different receiver number.
     */        
    case 5:
      nss.println("|| state5");
      
      order1.setState(2);  // set us up to look at next character
      break;
      
    } // end of state machine switch
    
    charPos ++; 
  }   // end of while loop
      
  nss.print( ">> start:" );
  nss.println( millis() );
  order1.setStartTime( millis() );   
     
}

///*
// * incrementMark increments the order mark by the specified amount,
// * preventing the order mark value from surpassing the maximum
// * length of the order array.
// */
//void incrementMark(byte amount) {
//  if ( order1.mark() == orderLen - 1 ) {
//    order0.mark = 0;
//    
//  } else {
//    order0.mark ++;
//  }
//}

///*
// * Continually checks timers and deactivates charges that have reached alloted time
// * @todo create charge status variable (consume less memory than array) for all charges so we can do an individual timer per charge
// */
//void deactivateCharges() {
//  digitalWrite(led, LOW);
//  nss.print("deactivating charge #");
//  for (byte c = 0; c <= sizeof(chargePin); c++) {
//    setCharge(c, 0);
//    nss.println(c);
//  }
//  nss.println();
//}
//
//void chargePause() {
//  
//}
