
// DESCRIPTION
/*
 @todo add to the description
 @todo make a fritzing chart
 
 The circuit:
   * LCD RS pin (4)  to digital pin 5
   * LCD RW pin (5)  to digital pin 6
   * LCD EN pin (6)  to digital pin 7
   * LCD D4 pin (11) to digital pin 8
   * LCD D5 pin (12) to digital pin 9
   * LCD D6 pin (13) to digital pin 10
   * LCD D7 pin (14) to digital pin 10
   
   * SoftwareSerial RX digital pin 2
   * SoftwareSerial TX digital pin 3
   
   * Keypad pin 1 to digital pin 11
   * Keypad pin 2 to digital pin 12
   * Keypad pin 3 to digital pin 14
   * Keypad pin 4 to digital pin 15
   * Keypad pin 5 to digital pin 16
   * Keypad pin 6 to digital pin 17
   * Keypad pin 7 to digital pin 18
   * Keypad pin 8 to digital pin 19

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
#include "SmartPayload.h"
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Adafruit_CharacterOLED.h>
#include <SimpleTimer.h>




// PIN CONNECTIONS
uint8_t ssRx = 2;                       // software serial RX
uint8_t ssTx = 3;                       // software serial TX
Adafruit_CharacterOLED lcd(4, 5, 6, 7, 8, 9, 10);  // OLED display
const byte ROWS = 4;                    // keypad rows
const byte COLS = 4;                    // keypad columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 11, 12, 15};  // connect to the row pinouts of the keypad
byte colPins[COLS] = {14, 16, 17, 18};  // connect to the column pinouts of the keypad





// CONFIG
boolean debugMode = 0;
const byte lcdWidth = 16;                // display width of LCD in characters
const byte lcdHeight = 2;                // display height of LCD in charaters
const int splashTime = 50;              // time the splash text will display
const char version[] = "DetPack v1";       // version string
const int ignitionHold = 1000;           // pause time after igntion before DIO pin goes back LOW
const byte receiverCount = 2;            // (zero index) number of receivers on our system. Can't be higher than 255.
const byte chargeCount = 2;              // (zero index) readable number of charges per receiver.s
//byte upArrow[8] = {
//  B00100,
//  B01110,
//  B11111,
//  B00100,
//  B00100,
//  B00100,
//  B00100
//};
//byte downArrow[8] = {
//  B00100,
//  B00100,
//  B00100,
//  B00100,
//  B11111,
//  B01110,
//  B00100
//};
byte thumbsUp[8] = {
  B00100,
  B01100,
  B01111,
  B11111,
  B11111,
  B11111,
  B01110 
};

// Menu system
char* menuOption[] = {
  "Main Menu",
  "Command",
  "Status",
  "FIRING SCRIPTS",
  "Strafe",
  "Counter-Strike",
  "C4 Mode",
  "DERP"
};

// Firing Scripts

// @todo this is not dynamic at all! fix this function so it is!
char* firingScriptTitle[] = {
  "Script1",
  "Script2",
  "STower",
  "Gas & Trench",
  "RPG",
  "Artillery",
  "Crazy Rockets!"
};


/*  Idea
 *
 *  display heading
 *  display option
 *  option is scrollable
 *  press enter
 *    option is now heading
 *    Are you sure?
 *    if press enter
 *      detonate
 *    else
 *      display heading
 *    
 */


/*
 * The Menu Code
 * 
 * menu has lots of parts
 * 
 *   - Menu (LCD) display
 *   - Menu input
 *   - Menu logic
 * 
 *
 *  I want to make the menu code a class
 *  lets reverse engineer classes
 *  so we can figure out how to make a class
 *  
 *  I'm using the MenuBackend library as an example.
 *  
 *  > Menu.cpp
 *    #include "Menu.h"      // including header file which contains all
 *                           // functions, both public and private.
 *    Menu::Menu(  void (*onMenuUser)(MenuItemInterface*) ){  // This is declaring something like a function inside the Menu class, however I'm not sure about the weired syntax.
 */





/* 
                                             menuActive
 *   Splash
 *   Main Menu                               menuActive[1]
 *     Command                                 menuActive[1][0]
 *     Status                                  menuActive[1][1]
 *     Firing Scripts                          menuActive[1][2]
 *       Script 1                                menuActive[1][2][0]
 *         Are you sure?                           menuActive[1][2][0][0]
 *           Ignition                                menuActive[1][2][0][0][0]
 *       Script 2                                menuActive[1][2][1]
 *         Are you sure?
 *           Ignition
 *       Script 3                                menuActive[1][2][2]
 *         Are you sure?
 *           Ignition
 *       Script 4                                menuActive[1][2][3]
 *         Are you sure?
 *           Ignition
 *       Script 5                                menuActive[1][2][4]
 *         Are you sure?
 *           Ignition
 *     Strafe                                  menuActive[1][3]
 *       Enter Interval
 *         Enter Charges
 *           Are you sure?
 *             Ignition
 *     Counter-Strike
 *       @todo
 *     C4 Mode
 *       @todo
 */

 



// Receiver addresses
uint32_t receiverAddress[receiverCount + 1][2] = {     // (3) receivers, with (2) addresses. (SL & SH)
  {0x0013a200, 0x408DD550},            // Receiver 0 (1)
  {0x0013a200, 0x409a1bc7},            // Receiver 1 (2)
  {0x0013a200, 0x408DD566}             // Receiver 2 (3)
};

/*
 * Receiver types
 *   0 is dumb remote
 *   1 is smart remote
 *   2 has yet to be invented
 *   3 has yet to be invented
 */
uint8_t receiverType[receiverCount + 1] = {
  1,                                   // Receiver 0 (1) is a smart receiver
  1,                                   // Receiver 1 (2) is a smart receiver
  0                                    // Receiver 2 (3) is a dumb receviver
};


// Charge DIO pins
//uint8_t chargePinCommand[receiverCount + 1][chargeCount + 1][2] = {  // (3) receivers, 3 charges per receiver. Number and order of charge pins can be re-mapped here.
//  {{'D', '0'}, {'D', '1'}, {'D', '2'}}                       // 
//};


uint8_t chargePinCommand[][3] = {
  {'D', '0'}, {'D', '1'}, {'D', '2'}
}; 

uint8_t chargePinHigh[] = { 0x5 };
uint8_t chargePinLow[] = { 0x4 };
 



//byte firingScript[][32] = {
//  {'Script1', 'R1-1'},
//  {'Script2', 'R2-1'},
//  {'STower', 'R3-1'},
//  {'Gas & Trench', 'R1-2', 'R2-2'},
//  {'Rocket', 'R1-3', 'P2', 'R2-3'},
//  {'Artillery', 'R1-1', 'P4', 'R1-2', 'P1', 'R2-1', 'P2', 'R2-2', 'P4', 'R2-3'}
//};

//
//byte firingScript[][32] = {
//  {0000},                    // Script1        R1-1
//  {0100},                    // Script2        R2-1
//  {0200},                    // STower         R3-1
//  {0001, 0101},              // Gas & Trench   R1-2 + R2-2
//  {0102, 9902, 0102},        // Rocket         R1-3, P2, R2-3
//  {0000, 9904, 0001, 9901, 0100, 9902, 0101, 9904, 0102} // Artillery
//};

char* textFiring = "--- FIRING! ---";

//
// INIT
//

int test = 0 ;

uint8_t command[4] = {0};                      // initial payload contents. This is changed by the program.

SoftwareSerial nss(ssRx, ssTx);
XBee xbee = XBee();
XBeeAddress64 address64 = XBeeAddress64(receiverAddress[0][0], receiverAddress[0][1]);  // SH + SL of your receiver radio
RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(address64, chargePinCommand[0], chargePinLow, sizeof(chargePinLow));  // Create a remote AT request with the IR command
RemoteAtCommandResponse remoteAtResponse = RemoteAtCommandResponse();                                             // Create a Remote AT response object
ZBTxRequest zbTx = ZBTxRequest(address64, command, sizeof(command));  // address, payload, size
SmartPayload smartPayload = SmartPayload();


Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// create arrays capable of 99 (100) remotes and 2 (3) charges
byte chargeStatus;                            // stores DIO pin status
//unsigned long chargeIgniteTime[99][2];      // stores the point in time the charge was ignited
//unsigned long chargeDeactivateTime[99][2];  // stores time that DIO pin needs to go LOW


SimpleTimer timerMenuSplash;
//SimpleTimer timer;                       // timer for everything
char cursorMove = 'n';

// MENU STUFF
boolean navLock = 0;                       // locks navigation. user can only make choices.
byte menuActive = 0;                       // the active menu. 1 is splash screen
boolean bigChoice = 0;                       // indicates if the user has a big choice to make (fire or don't)


void setup() { 
  // if debug mode is off, enable software serial.
  // otherwise, those pins will be used as a switch
  
  if(debugMode == 0){
    nss.begin(9600);
  }
  
  Serial.begin(9600);                                // begin Serial comms for xbee
  xbee.begin(Serial);                                // begin xbee library
  lcd.createChar(0, thumbsUp);
  lcd.begin(lcdWidth, lcdHeight);                    // start lcd
  lcd.setCursor(0, lcdHeight - 1);                   // set lcd cursor position to second line
  lcd.setCursor(0,0);
  notify1(menuOption[3]);                            // display initial heading
  notify2(firingScriptTitle[menuActive]);            // set active firing script

  
  // create timer object that will end display of the splash screen
  // and move to the menu screen.
  timerMenuSplash.setTimeout(splashTime, splashStop);
//  lcd.createChar(0, upArrow);
//  lcd.createChar(1, downArrow);  
  
  
 // When powered on, XBee radios require a few seconds to start up
 // and join the network.
 // During this time, any packets sent to the radio are ignored.
 // Series 2 radios send a modem status packet on startup.
 
 // it took about 4 seconds for mine to return modem status.
 // In my experience, series 1 radios take a bit longer to associate.
 // Of course if the radio has been powered on for some time before the sketch runs,
 // you can safely remove this delay.
 // Or if you both commands are not successful, try increasing the delay.
 
// delay(5000);
}

void loop() {
  
  
  keypadCheck();
  //timerMenuSplash.run();
  
  //menuLogic();

  
  xbeeCheckRx();
  //deactivateCharges();
  //nss.println("looping.");
}


/*
 * Checks whether the receiver is on or off
 * @todo delete this function if it's not used.
 */
void checkReceiver(byte receiver) {
  
  // set appropriate receiver destination address
  nss.println("setting receiver destination");
  address64.setMsb(receiverAddress[receiver][0]);
  address64.setLsb(receiverAddress[receiver][1]);
  remoteAtRequest.setRemoteAddress64(address64);
  zbTx.setAddress64(address64);

  // status info
  nss.println("checking receiver status.");
  
  // set the DIO pin HIGH (AT command HEX value 0x5)
  nss.println("setting DIO pin high");
  remoteAtRequest.setCommandValue(chargePinHigh);
  remoteAtRequest.setCommandValueLength(sizeof(chargePinHigh));


  // send command
  nss.println("Sending remote at request to get status.");
  xbee.send(remoteAtRequest);
}


/*
 * Quick and dirty hack of a menu to sell the effect
 * @todo get a better menu, you lazy programmer!
 *
 * menuActive
 * menuScript
 * 
 * Firing Scripts
 *  Script1             menuScript = 0
 *    are you sure
 *      ignite
 *  Script2             menuScript = 1
 *    are you sure
 *      ignite
 *  Script 3            menuScript = 2
 *    are you sure      
 *      ignite
 *   
 */
void menuDisplay(byte menu, byte option){
  switch(menu) {
    case 0:
      // print nothing
      break;
      
    case 1:
      // splash?
      lcd.println(version);
      //menuActive = 0;
      break;
      
    case 2:
      // firescript menu
      notify1(menuOption[3]);
      //menuActive = 0;
      
      break;
  }
  
  switch(option) {
    default:
      notify2(firingScriptTitle[option]);
  }   
      
}


// if navMenu('e')


//   if menuScript == 0
//     areYouSure?

//     if navMenu('e')
//       fireScript(0);
//
//     else if navMenu('#')
//       make script1 active


//  

// 
//
//
//byte parseFiringScript(byte script) {
//  //yte scriptString = firingScript[script];       // the string of text to parse
//  
//  
//  for(int character = 0; character < sizeof(firingScript[script]); character ++) {
//    // check if it's an 'R' or a 'P'
//    if(firingScript[0][character] == 'R') {
//      nss.print("I found an R at position ");
//      nss.println(character);
//    } else if(firingScript[0][character] == 'P') {
//      nss.print("I found a P at position ");
//      nss.println(character);
//    } else {
//      nss.print("I found no character I recognize.");
//    }    
//    
//    
//  }
//
//  // turn R1-1 into setCharge(0, 0, 1) 
//  //   
// 
// 
// 
// 
//  
//}


/*
 * Starts the timer that holds the DIO line HIGH momentarily
 */
void igniteHoldTimer(byte receiver, byte charge) {
  nss.println("TIMER engage");
  
  
  
  //chargeStatus = chargeStatus >> receiver
  
  
  //chargeStatus = 
}


/*
 * Continually checks timers and deactivates charges that have reached alloted time
 */
void deactivateCharges() {
  
    // find active charges
  //   - iterate through chargeStatus
  //   - if there's a 1 in [x], check ignitiontime of [x]
  //   - if millis() - [x] > ignitionHols     // see if time is expired to deactivate
  // if time is expired, deactivate

  

  int receiver;  // iterate through remotes
  for(receiver = 0; receiver <= receiverCount; receiver ++) {
    int charge;  // iterate through charges on remote
    for(charge = 0; charge <= chargeCount; charge ++) {
      //nss.print("checking R");
      //nss.print(receiver);
      //nss.print("-");
      //nss.println(charge);
//      if(millis() - chargeIgniteTime[receiver][charge] > ignitionHold) {   
     // }
    }
  // when charge is ignited
  //   set its ignite time
  //   set it's deactivate time
  // check regularly for deactivate times reached
  // if deactivate time is reached
  //   deactivate
  
  }
}


/*
 * Continually checks for xbee receipt packets.
 * When one is received, it is processed accordingly.
 * 
 */
void xbeeCheckRx() {
  
  
  
  // @todo use the modem status response packet... 
  // @todo ...to determine which receivers are associated
  
  
  // read the packet in the buffer (if any)
  xbee.readPacket();
  
  if (xbee.getResponse().isAvailable()) {
    // got something
    
    if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE) {
      // got a AT command response packet (yay, that's what we want!)
      nss.println("Good! We got an AT response packet");
      
      // fill our remote AT rx packet
      xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);
      
      // find out if DIO state was triggered successfully
      if (remoteAtResponse.isOk()) {
        // it was
        nss.println("Good! DIO pin state was triggered successfully.");
        
      } else {
        // it wasn't
        nss.println("Bad! DIO pin state was NOT triggered");
      }
      
    } else {
      // We did not get an AT command response packet
      nss.print("Instead of an AT response packet, we got: ");
      nss.println(xbee.getResponse().getApiId(), HEX);
      
      // if we got frame 0x91
      if (xbee.getResponse().getApiId() == 0x91) {
        nss.println("Oh boy, we got an Explicit Transmit API frame!");
//        xbee.getResponse().getZBRxResponse(rx);
//        
//        if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
//          // if sender got an ACK
//          nss.println("zb packet ack");
//          
//        } else {
//          // we got it but sender didn't get an ack
//          nss.println("zb packet NOACK");
        }
     }
        
  } else if (xbee.getResponse().isError()) {
    // there was an error in the response packet
    nss.print("Error reading packet. Error code: ");
    nss.println(xbee.getResponse().getErrorCode());
  
  } else {
    // no response at all
    //nss.println("No response from radio");
  }
  
  
//  if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
//    // Did we receive a modem status response packet?
//    // the local xbee sends this response on certain events, like association/dissociation
//    if (msr.getStatus() == ASSOCIATED) {
//      // modem is associated
//      nss.println("MODEM STATUS: Associated");
//      
//    } else if (msr.getStatus() == DISASSOCIATED) {
//      // modem is disassociated
//      nss.println("MODEM STATUS: Disassociated");
//      
//    } else {
//      // another status?
//      nss.println("MODEM STATUS: Something... Who know's what!");
      
}

/*
 * Set the status of a charge.
 * @param byte remote is the remote number to set
 * @param byte charge is the charge number to set
 * @param bool status sets the status. 1 is DIO HIGH (ignite), 0 is DIO LOW (off)
 */
void setCharge(byte receiver, byte charge, bool setStatus) {
  
  // set appropriate receiver destination address
  nss.println("setting receiver destination");
  address64.setMsb(receiverAddress[receiver][0]);
  address64.setLsb(receiverAddress[receiver][1]);
  
  
  // determine if target receiver type (smart or dumb, etc.)
  //nss.print("testing against receiver number: '");
  //nss.print(receiver);
  //nss.print("' which is of type: ");
  //nss.println(receiverType[receiver]); @todo deleteme
  
  switch (receiverType[receiver]) {
    case 0:
    // target receiver is dumb receiver. Send AT commands
    // update the object with the correct address
    remoteAtRequest.setRemoteAddress64(address64);
    
    nss.println("sending to dumb receiver");
  
    // set appropriate DIO pin
    nss.println("setting dio pin");  
    remoteAtRequest.setCommand(chargePinCommand[charge]);
  
    if(setStatus == 1) {
      // if we are igniting
      nss.println("we are igniting.");
      
      // set the DIO pin HIGH (AT command HEX value 0x5)
      nss.println("setting DIO pin high");
      remoteAtRequest.setCommandValue(chargePinHigh);
      remoteAtRequest.setCommandValueLength(sizeof(chargePinHigh));
  
    
      // send command
      nss.println("Sending remote at request.");
      xbee.send(remoteAtRequest);
      // set ignite time to now  
      //chargeIgniteTime[receiver][charge] = millis();
    
      // set charge status to 1
      //chargeStatus[receiver][charge] = 1;
    
      
      // set ignite time to now
      // chargeIgniteTime[remote][charge] = millis();
  
      // set charge status to 1
      //  chargeStatus[remote][charge] = 1;
  
    
      
    } else {
      // we are deactivating
      nss.println("we are deactivating");
   
      // set the DIO pin HIGH (AT command HEX value 0x5)
      //nss.println("setting DIO pin low");
      remoteAtRequest.setCommandValue(chargePinLow);
      remoteAtRequest.setCommandValueLength(sizeof(chargePinLow));
  
      // send command
      //nss.println("Sending remote at request.");
      xbee.send(remoteAtRequest);  
    }
    break;
    
  case 1:
    // target receiver is a smart receiver. Send serial messages.

    // update the object with the correct target address
    zbTx.setAddress64(address64);
    // @todo thumbs up icon for ACK
    // ccc
    
    if(setStatus) {
      // we're activating
      nss.print("smartpay: ");
      nss.println(smartPayload.requestIgnite(charge, 1, 1));
      
      // chunk-of-payload = chunk of smartpayload
      command[0] = smartPayload.requestIgnite(charge, 1, 0);    
      command[1] = smartPayload.requestIgnite(charge, 1, 1);    
      command[2] = smartPayload.requestIgnite(charge, 1, 2);    
      command[3] = smartPayload.requestIgnite(charge, 1, 3);    
   
    } else {
      // we're de-activating
 
      command[0] = smartPayload.requestIgnite(charge, 0, 0);    
      command[1] = smartPayload.requestIgnite(charge, 0, 1);    
      command[2] = smartPayload.requestIgnite(charge, 0, 2);    
      command[3] = smartPayload.requestIgnite(charge, 0, 3);      
    }
        
    // debug console
    nss.print("command shrunkified   : ");
    for (int character = 0; character < sizeof(command); character++) {
      nss.print(command[character]);
      nss.print(",");
    }
    nss.print(command[sizeof(command)]);
    nss.println(".");
    
    // send the payload    
    xbee.send(zbTx);
      
    break;  // out of smart receiver case

    
  default:
    nss.println("target receiver type is not supported");
    
  } 
}
      

void fireScript(byte scriptNumber) {
  switch(scriptNumber) {
    case 0:
      // Script1
      // R2-2
      notify1(textFiring);
      notify2(firingScriptTitle[scriptNumber]);
      setCharge(1, 2, 1);  // R2-1 on
      setCharge(1, 3, 1);
      setCharge(1, 4, 1);
      setCharge(1, 5, 1);
      setCharge(1, 6, 1);
      setCharge(1, 1, 1);  // R2-2 on
      setCharge(1, 0, 0);  // R2-0 on
      
      
      

      
      break;
    
    case 1:
      // fire script 2
      // R3-(1..3)
      notify1(textFiring);
      notify2(firingScriptTitle[scriptNumber]);
      setCharge(2, 0, 1);  // R3-1
      setCharge(2, 1, 1);  // R3-2
      setCharge(2, 2, 1);  // R3-3
      break;
    
    case 2:
      // fire script 3
      notify1(textFiring);
      notify2(firingScriptTitle[scriptNumber]);
      setCharge(2, 0, 1);  // R3-1
      break;
    
    case 3:
      // fire script 4
      notify1(textFiring);
      notify2(firingScriptTitle[scriptNumber]);
      setCharge(0, 1, 1);
      setCharge(1, 1, 1);
      break;
      
    case 4:
      // fire script 5
      notify1(textFiring);
      notify2(firingScriptTitle[scriptNumber]);
      setCharge(1, 2, 1);
      delay(1000);
      setCharge(2, 2, 1);
      break;
      
    
    case 5:
      // fire script 6 (artillery)
      notify1(textFiring);
      notify2(firingScriptTitle[scriptNumber]);
      
      
      setCharge(0, 0, 1);  // R1-1   rocket1 launch
      delay(4000);
      setCharge(0, 1, 1);  // R1-2   rocket2 launch
      delay(1000);     
      setCharge(1, 0, 1);  // R2-1   rocket1 impact
      delay(2000);
      setCharge(0, 2, 1);  // R1-3   rocket3 launch
      delay(1000);
      setCharge(1, 1, 1);  // R2-2   rocket2 impact
      delay(2000);
      setCharge(1, 2, 1);  // R2-3   rocket 3 impact
      break;

    case 6:
      // fire script 7 (crazy rockets!)
      notify1(textFiring);
      notify2(firingScriptTitle[scriptNumber]);
      
      setCharge(0, 0, 1);  // R1-1   rocket1 launch
      //delay(100);
      setCharge(0, 1, 1);  // R1-2   rocket2 launch
      //delay(100);     
      setCharge(0, 2, 1);  // R1-3   rocket3 launch
      setCharge(0, 3, 1);
      setCharge(0, 4, 1);
      setCharge(0, 5, 1);
      //delay(1000);
      
      setCharge(1, 0, 1);  // R2-1   rocket1 impact
      setCharge(2, 0, 1);  // R3-1
      //delay(100);
      setCharge(1, 1, 1);  // R2-2   rocket2 impact
      setCharge(2, 1, 1);  // R3-2
      //delay(100);
      setCharge(1, 2, 1);  // R2-3   rocket 3 impact
      setCharge(2, 2, 1);  // R3-3
      break;
      

    case 9:
      // clear all
      notify1("clearing!");
      
      
      for(byte receiver = 0; receiver <= receiverCount; receiver ++){
        for(byte charge = 0; charge <= chargeCount; charge ++) { 
          setCharge(receiver, charge, 0);  // deactivate charge on receiver
          nss.print("deactivating R");
          nss.print(receiver);
          nss.print("-");
          nss.println(charge); 
     
        }
      }
      break;  
    
    default:
      nss.println("firescript number inputted is out of range");
  }
  notify1(menuOption[3]);
}

 
/*
 * called by a timer to stop the splash screen.
 * function does this by changing the global variable, 'menuActive' to 1
 * effectively changing the active menu from splash to main
 */
void splashStop() {
   menuActive = 2;
   int i = 0;
   while (i<10) {
     
     nss.println("DERP DERP DERP DERP DERP");
     i++;
   }
}



/*
 * Function to navigate the menu.
 * @param char navDirection accepts 'u', 'd', 'l', 'r' for up, down, left, right, respectively
 */
void navigateMenu(char navInput) {
  switch (navInput) {
    case 'd':
      if(navLock == 0) {
        // move down the menu
        if(menuActive == (sizeof(menuOption)/sizeof(menuOption[0]) - 2)) {
          // if we're at the last menu option, loop to the first menu option
          menuActive = 0;
  
        } else {
          // go to the next menu item
          menuActive ++;
        }
        menuDisplay(0, menuActive);                // tell program that the menu option needs to be updated
      } else {
        nss.println("navLock.");
      }
      nss.print("active menu: ");
      nss.println(menuActive);      
      break;
      
    case 'u':
      // move up the menu
      if(navLock == 0) {
        if(menuActive == 0) {
          // if we're at the first menu option, loop to the last menu option
          menuActive = (sizeof(menuOption)/sizeof(menuOption[0]) - 2);
          
        } else {
          // go to the previous menu item
          menuActive --;
  
        }      
        menuDisplay(0, menuActive);
      } else {
        nss.println("navLock.");
      }
      nss.print("active menu: ");
      nss.println(menuActive);
      break;
      
    case 'l':
      if(navLock == 0) {
        // move left on the menu
        menuDisplay(0, menuActive);              // tell program that the menu option needs to be updated
      } else {
        nss.println("navLock.");
      }
      break;
      
    case 'r':
      if(navLock == 0) {
      // move right on the menu
      menuDisplay(0, menuActive);                // tell program that the menu option needs to be updated
      } else {
        nss.println("navLock");
      }
      break;
      
    case 'e':
      // enter on the menu
      //   - new active menu is $(activeMenuOption)
      
      // if user has been asked to confirm and they just said yes
      if(areYouSure(-1)) {
        // fire the script, unlock controls, and reset bigChoice
        fireScript(menuActive);
        navLock = 0;
        bigChoice = 0;
        
      } else {
        // ask them if they are sure, and lock controls until they answer.
        areYouSure(menuActive);
        navLock = 1;
      }
      //if(areYouSure
      

      
      
      // if areYouSure is on screen
      //   fireScript(menuActive);
      // else
      //   areYouSure
      //   navLock = 1
      //

      navLock = 0;   // unlock navigation if it was locked.
      break;
      
    case 'b':
      // back on the menu
      navLock = 0;   // unlock navigation if it was locked.
      bigChoice = 0; 
      notify1(menuOption[3]);
      notify2(firingScriptTitle[menuActive]);
      break;
  }
}


/*
 * Asks the user if they are sure to execute the action.
 * Accepts a fireScript number as a parameter
 * if no parameter is given, function will
 * return 1 if user has been asked if they're sure
 * return 0 if user has not been asked if they're sure
 * 
 * @param scriptNum byte
 * @return boolean
 */
boolean areYouSure(char scriptNum) {

  // if function was called with a positive parameter
  // (negative means return whether user has a choice to make or not)
  if(scriptNum >= 0) {
    // ask user if they're sure
    notify1(firingScriptTitle[scriptNum]);
    notify2("Are you sure?");
    
    // set the global variable to show that the user has a big choice to make
    // (during this time, the user shouldn't be able to navigate, only choose yes or no)
    bigChoice = 1;
    
  } else {
    // return a value so the code calling this function knows if
    // if the user has a big choice to make
    
    if(bigChoice == 1) {
      // user has to make a choice
      return 1;
      
    } else {
      // user does not have to make a choice
      return 0;
    }
  }
}

void keypadCheck(){
  char key = kpd.getKey();
  if(key != NO_KEY) {
    
    
    switch (key) {
      case '2':
        // if 2 is pressed, send up command
        navigateMenu('u');
        
        break;
        
      case '4':
        // if 4 is pressed, send left command
        //navigateMenu('l');
        break;
        
      case '6':
        // if 6 is pressed, send right command
        //navigateMenu('r');
        break;
        
      case '8':
        // if 8 is pressed, send down command
        navigateMenu('d');
        break;
       
             
      case 'A':
        // if A is pressed, do stuff
        //fireScript(4);
        break;
      
      case 'B':
        // if B is pressed, do stuff
        //fireScript(5);
        break;
        
      case 'C':
        // if C is pressed, do stuff
        fireScript(9);
        break;
      
      case 'D':
        // if D is pressed, do stuff
        //nss.println("FIRING EXPERIMENTAL!");
        //fireScript(5);
        break;
    
      case '*':
        // if star is pressed, send back command
        navigateMenu('b');
        break;
    
      case '#':
        // if pound is pressed, send enter command
        navigateMenu('e');
        break;
    }  
    
    // On the serial monitor, print the key that was pressed.
    nss.print("we got a key: ");
    nss.println(key);
  }
}


void debugXbee(uint32_t info) {
  nss.println(info, HEX); 
}


char notify1(char* message) {
  lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 0);
  lcd.print(message); 
  nss.println(message);
}

char notify2(char* message) {
  lcd.setCursor(0, 1);
  lcd.print("                "); 
  lcd.setCursor(0, 1);
  lcd.print(message); 
  nss.println(message);
}
