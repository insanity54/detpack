
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


// PIN CONNECTIONS
uint8_t ssRx = 11;                         // software serial RX
uint8_t ssTx = 12;                         // software serial TX
uint8_t led = 13;                          // status LED
uint8_t chargePin[] = { 2, 3, 4, 5, 6, 7}; // charges

// CONFIG
byte me = 2; // zero-index of this receiver's number. @todo make this automatic. see below or search for, "automatic receiver id"
//boolean debugMode = 0;
//unsigned int ignitionHold = 2000;           // pause time after igntion before DIO pin goes back LOW

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
SimpleTimer timer;
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
}

void loop() {
  // check xbee for commands
  xbeeCheck();
  timer.run();
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
      parseData();

      
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

/*
 * Parses the data from the transmitter
 * validates the data
 * and processes it accordingly.
 */
void parseData() {
  // find the command id
  // exec the first set
  // find the second set
  // find the third set
  
  // find command id
  int commandId;
  commandId = rx.getData(0);
  commandId = commandId * 100 + rx.getData(1); // concatenate data0 with data1
  nss.print("commandId:");
  nss.println(commandId);
  
  // find first set
  byte pak[2] = {0};
  pak[0] = rx.getData(2);
  pak[1] = rx.getData(3);
  
  switch (commandId) {
  case 6865:
    // 'DA' De-activate command
    nss.println("de-activating");
    for (byte charge = 0; charge <= sizeof(chargePin); charge++) {
      setCharge(charge, 0);
    }
    break;    
    
  case 7083:
    // 'FS' Fire Script command
    nss.print(">> FS:");
    nss.print(pak[0]);
    nss.print(",");
    nss.println(pak[1]);
    
    if (pak[0] == me) {
      setCharge(pak[1], 1); // set the charge number HIGH
      nss.print("ignite:");
      nss.println(pak[1]);
    }
    break;
    
  default:
    // Unsupported command
    nss.print("unsuportd cmd");

    // @todo add more modes. Fun!
  }
}

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
 * Continually checks timers and deactivates charges that have reached alloted time
 * @todo create charge status variable (consume less memory than array) for all charges so we can do an individual timer per charge
 */
void deactivateCharges() {
  digitalWrite(led, LOW);
  nss.print("deactivating charge #");
  for (byte c = 0; c <= sizeof(chargePin); c++) {
    setCharge(c, 0);
    nss.println(c);
  }
  nss.println();
}

void chargePause() {
  
}
