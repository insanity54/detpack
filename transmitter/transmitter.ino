
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
       
   * Numbers used in code start at zero, so Receiver 1 away from keyboard is Receiver 0 in code.
     They are converted to human readable numbers (R0 is now R1) right before being displayed.
     * It's only Receiver 1 to the user. It's actually Receiver 0!!!

  
  @todo
  Known Issues 
    * receivers use satan function 'delay()'
    * receivers can become overloaded with data and drop packets (probably because of satan)
    * clear command sometimes does not clear any receivers
    * sending commands over the air always sends 24 bytes, even when the command is only 4 bytes

 */


// INCLUDE
#include <XBee.h>
#include "SmartPayload.h"
//#include "SweetMenu.h" @todo make the friggin' menu already!
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
const byte led = 13;


// CONFIG
boolean debugMode = 1;
const byte lcdWidth = 16;                 // display width of LCD in characters
const byte lcdHeight = 2;                 // display height of LCD in charaters
const int splashTime = 50;                // time the splash text will display
//const char version[] = "DetPack v0.10a";  // version string
const int ignitionHold = 1000;            // pause time after igntion before DIO pin goes back LOW
const byte receiverCount = 2;             // (zero index) number of receivers on our system. Can't be higher than 255.
const byte chargeCount = 3;               // (zero index) readable number of charges per receiver.s


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
byte thumbsDown[8] = {
  B01110,
  B11111,
  B11111,  
  B11111,
  B01111,
  B01100,
  B00100  
};
byte lock[8] = {
  B01110,
  B10001,
  B10001,
  B11111,
  B11011,
  B11011,
  B11111 
};

//byte key[8] = {
//  B00100,
//  B01010,
//  B00100,
//  B00100,
//  B00100,
//  B00110,
//  B00110
//};
// Menu system  // @todo implement these other options
char* menuOption[] = {
//  "MAIN",
//  "COMMAND",
//  "STATUS",
  "SCRIPTS",
//  "STRAFE",
//  "CSTRIKE",
//  "C4 MODE",
//  "DERP"
};

// Firing Scripts
char* firingScriptTitle[] = {
  "Arty Taco1",
  "Arty Taco2",
  "Arty Taco3",
  "Arty Olympic1",
  "Arty Olympic2",
  "Arty Olympic3",
  "Olympic4"
};



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




// Receiver addresses
unsigned int receiverAddress[receiverCount + 1][2] = {     // (3) receivers, with (2) addresses. (SL & SH)
  {0x0013a200, 0x408DD550},            // Receiver 0 (1)
  {0x0013a200, 0x409a1bc7},            // Receiver 1 (2)
  {0x0013a200, 0x408DD566}             // Receiver 2 (3)
};

/*
 * Receiver types
 *   0 is dumb receiver
 *   1 is smart receiver
 *   2 has yet to be invented
 *   3 has yet to be invented
 */
uint8_t receiverType[receiverCount + 1] = {
  1,                                   // Receiver 0 (1) is a smart receiver
  1,                                   // Receiver 1 (2) is a smart receiver
  1                                    // Receiver 2 (3) is a smart receviver
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
 
byte rangeCheckCommand[] = {'N', 'D'}; // a harmless AT command to use to check if a receiver is in range.

/*
 * Firing Script Definitions
 *
 * 120 is pause milliseconds
 * 121 is pause seconds
 * 122 is pause minutes
 *
 * 128 needs to be EOL delimeter
 * 
 */
byte firingScript[][9] = {
  {1,0,121,5,0,1,128},                 // Arty Taco1
  {1,1,121,5,0,2,128},                 // Arty Taco2
  {1,2,121,5,0,3,128},                 // Arty Taco3
  
  {1,3,121,5,2,0,128},                 // Arty Olympic1
  {1,4,121,5,2,1,128},                 // Arty Olympic2
  {1,5,121,5,2,2,128},                 // Arty Olympic3
  {2,3,128}                            // Olympic4
  
//  {0,0,128},                         // Gas1
//  {0,1,128},                         // Gas2
//  {0,2,128},                         // Gas3
//  {1,0,128},                         // FOB1
//  {1,1,128},                         // FOB2
//  {1,2,128},                         // FOB3
//  {1,3,128},                         // FOB4
//  {1,4,128},                         // FOB5
//  {1,5,128},                         // FOB6
//  {2,0,128},                         // Olympic1
//  {2,1,128},                         // Olympic2
//  {2,2,128}                          // Olympic3
};


char* textFiring = "firing!";

//
// INIT
//

uint8_t command[24] = {0};                      // initial payload contents. This is changed by the program.

SoftwareSerial nss(ssRx, ssTx);
XBee xbee = XBee();
XBeeAddress64 address64 = XBeeAddress64(receiverAddress[0][0], receiverAddress[0][1]);  // SH + SL of your receiver radio
RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(address64, chargePinCommand[0], chargePinLow, sizeof(chargePinLow));  // Create a remote AT request with the IR command
RemoteAtCommandResponse remoteAtResponse = RemoteAtCommandResponse();                                             // Create a Remote AT response object
ZBTxRequest zbTx = ZBTxRequest(address64, command, 24);  // address, payload, size
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
SmartPayload smartPayload = SmartPayload();


SimpleTimer timer;                         // timer object
// create timer id that will end display of the splash screen
// create timer that will clear the padlock icon after 1s
int timerNavLock = timer.setTimeout(3000, menuClearLock);


Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// create arrays capable of 99 (100) receivers and 2 (3) charges
//byte chargeStatus;                            // stores DIO pin status
//unsigned long chargeIgniteTime[99][2];      // stores the point in time the charge was ignited
//unsigned long chargeDeactivateTime[99][2];  // stores time that DIO pin needs to go LOW


// MENU STUFF
boolean navLock = 0;                       // locks navigation. user can only make choices.
boolean uiLock = 0;                        // completely locks user interface. activated by keyswitch
byte menuActive = 0;                       // the active menu. 1 is splash screen
boolean bigChoice = 0;                     // indicates if the user has a big choice to make (fire or don't)


void setup() { 
  // if debug mode is off, enable software serial.
  // otherwise, those pins will be used as a switch
  
  if(debugMode){
    // debug mode is on so enable software serial
    nss.begin(9600);
    
  } else {
    // debug mode is off so disable software serial, enable navlock switch
    pinMode(ssRx, INPUT);
    pinMode(led, OUTPUT);
  }
  
  Serial.begin(9600);                                // begin Serial comms for xbee
  xbee.begin(Serial);                                // begin xbee library
  lcd.createChar(0, thumbsUp);
  lcd.createChar(1, thumbsDown);
  lcd.createChar(2, lock);
  lcd.begin(lcdWidth, lcdHeight);                    // start lcd
  lcd.setCursor(0, lcdHeight - 1);                   // set lcd cursor position to second line
  lcd.setCursor(0,0);

  menuDisplay();
  
  
}

void loop() {  
  uiLockCheck();
  keypadCheck();

  timer.run(); // governs various timers
  
  xbeeCheckRx();
}

void uiLockCheck() {
 
  if (!debugMode) {
    // debug mode is off
    
    if (!uiLock == digitalRead(ssRx)) {
      // if state just changed
      
      if (digitalRead(ssRx)) {
        // switch is closed, lock.
        nss.println("switch closed");
        lcd.clear();
        menuDisplayLock();
        digitalWrite(led, HIGH);
        uiLock = 1;
        
      } else {
        // switch is open, unlock.
        nss.println("switch open");
        menuDisplay();
        digitalWrite(led, LOW);
        uiLock = 0;
      }
    }
  }
}


void menuDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(menuOption[0]);
  lcd.setCursor(0, 1);
  lcd.print(firingScriptTitle[menuActive]);
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
   
    switch (xbee.getResponse().getApiId()) {
      
      case REMOTE_AT_COMMAND_RESPONSE:
        // got an AT command response packet
        nss.println("rx atrx pak");
      
        // fill our remote AT rx object
        xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);
      
        // find out if DIO state was triggered successfully
        if (remoteAtResponse.isOk()) {
          // it was
          nss.println("DIO gud");
        
        } else {
          // it wasn't
          nss.println("DIO bad");
        }
        break;
        
      case ZB_TX_STATUS_RESPONSE:
        // tx is completed, receiver replies with this.
        // indicates successful or failed transmission.
        
        // fill our status response object
        xbee.getResponse().getZBTxStatusResponse(txStatus);
        
        // get delivery status (the fifth byte)
        if (txStatus.getDeliveryStatus() == SUCCESS) {
          // successful delivery
          // @todo add action for correct receiver
          // @tod re-add thumbsdown feature
          //lcd.setCursor(10, 0);
          //lcd.write(0);
          //nss.println("succee"); // @todo track down bug that is keeping this from failing when it should
          
          
        } else {
          // failed delivery
          // @todo re-implement thumbsdown feature
          // @todo add action for failure that relates to correct receiver
          //lcd.setCursor(10, 0);
          //lcd.write(1);
          //nss.println("faile");
        }
        break;
             
      case 0x91:
        // zb explicit tx api frame (0x91) (unsupported)
        nss.println("xplicit tx api frame");
        break;
 
      default:
        // received neither 0x90 or 0x91
        nss.print("rx frame: ");
        nss.println(xbee.getResponse().getApiId(), HEX);
      }
    
  } else if (xbee.getResponse().isError()) {
    // there was an error in the response packet
    nss.print("Error code: ");
    nss.println(xbee.getResponse().getErrorCode());
  
  }
}

/*
 * Set the status of a charge either on or off. For dumb receivers only.
 * @param byte receiver is the receiver number to set
 * @param byte charge is the charge number to set
 * @param bool status sets the status. 1 is DIO HIGH (ignite), 0 is DIO LOW (off)
 */
void setCharge(byte receiver, byte charge, bool setStatus) {
  
  // set appropriate receiver destination address
  nss.println("set rx dest");
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
    
    nss.println("send dumb rxr");
  
    // set appropriate DIO pin
    nss.println("set dio pin");  
    remoteAtRequest.setCommand(chargePinCommand[charge]);
  
    if(setStatus == 1) {
      // if we are igniting
      nss.println("IG");
      
      // set the DIO pin HIGH (AT command HEX value 0x5)
      nss.println("set DIO 1");
      remoteAtRequest.setCommandValue(chargePinHigh);
      remoteAtRequest.setCommandValueLength(sizeof(chargePinHigh));
  
    
      // send command
      nss.println("send remAT req");
      nss.println("  send pay");
      xbee.send(remoteAtRequest);

    
      
    } else {
      // we are deactivating
      nss.println("deact");
   
      // set the DIO pin HIGH (AT command HEX value 0x5)
      //nss.println("setting DIO pin low");
      remoteAtRequest.setCommandValue(chargePinLow);
      remoteAtRequest.setCommandValueLength(sizeof(chargePinLow));
  
      // send command
      //nss.println("Sending remote AT request.");
      nss.println("   send pay");
      xbee.send(remoteAtRequest);  
    }
    break;

  case 1:
    // target receiver is a smart receiver. Send serial messages.
    nss.println("dont send dumb to smart!");
    break;


  default:
    nss.println("target rxr type not supp");
    
  }
}
      
/*
 * smartClear sends a messge to smart receivers, essentially saying, "clear everything."
 * It is "smart" in the sense that it only has to send one message to
 * the receivers which tells the receivers what to do. The "dumb" version of this is telling
 * the receiver, "R1-0 off. R1-1 off. R1-2 off. R1-3 off." whereas this smart version says, "turn everything off."
 * Only smart receivers support this type of clearing.
 */
void smartClear() {
  
  memset(command, 0, sizeof(command));  // clear command array (@todo fix this dirty hack)
  
  command[0] = 68;  // D
  command[1] = 65;  // A
  command[2] = 128; // 128 is our end-of-line delimiter
  
  address64.setMsb(0x00000000);
  address64.setLsb(0x0000FFFF);
  
  zbTx.setAddress64(address64);

  // send the payload
  nss.println(" )) clr pay");
  xbee.send(zbTx);
}
      
/*
 * determines the size of the command payload so we don't send any extra bytes after the 128
 */
void smartSize() {
  
}


/*
 * smartFire validates a firing script set in the firingScript array,
 * then determines what to do with the commands in the script.
 * a command payload is assembled and stored as a char array in 'command'
 * Finally, the command is broadcast via the XBee to all XBees in the same PAN
 *
 * @param byte scriptNumber the script number from the 'firingScript' array
 */
void smartFire(byte scriptNumber) {
  //ccc
  
  // The first two characters of our command payload are 'FS' which is an identifier of
  // the type of command we are sending. 'FS' tells the receiver that this command
  // is a Fire Script command.
  // @todo make this set dynamically so it works for commands other than 'FS' (?)
  command[0] = 70;  // F
  command[1] = 83;  // S
  
  nss.println("fire script");

  for (int charPos = 0; charPos < sizeof(firingScript[scriptNumber]); charPos++) {
    // sizeof(firingScript[0]) will equal x: firingScript[][x]
    // Here we are going through each char of the selected firingScripts array.
    // i.e. we are figuring out what the firing script is telling us to do.
    // we will see a receiver number follwed by a charge number,
    // or a special designator (a number above 100) which tells us to pause.
          
    if (firingScript[scriptNumber][charPos] == 128) {
      // if the char we are lookig at is 128, stop checking this script.
      // 128 is an end-of-line (EOL) designator
      
      // add the EOL designator to the command payload
      // offset by +2 because the first two of command payload are 'F' and 'S'
      command[charPos+2] = 128;
      break;  // stop adding chars because 128 indicated the end of the script
      
    } else {
      // char is not 128
      // add the number to the command payload
      // offset by +2 because the first two of command payload are 'F' and 'S'
      
      nss.print("charPos:");
      nss.print(charPos);
      nss.print(" character:");
      nss.println(firingScript[scriptNumber][charPos]);
      
      // @todo this line causes a crash when either writing command[24] or accessing firingScript[scriptNumber][24]
      //       worked around for now by preventing it from writing to [24]
      if (charPos < 22) {
        command[charPos+2] = firingScript[scriptNumber][charPos];
        
      } else {
        nss.println("Cmd too long. All FS must end in 128");
      }
    }
  }
  
  // @todo clean this up
  nss.println("smart set dest");
  //for (int r = 0; r < receiverCount; r++) {
  // send orders to each receiver
  // @todo either make this a broadcast, or determine
  //       which receivers actually need the orders
  //       (If R3 is not in the fire script, send just to R1 & R2)
  //    address64.setMsb(receiverAddress[r][0]);
  //    address64.setLsb(receiverAddress[r][1]);
  
  address64.setMsb(0x00000000);
  address64.setLsb(0x0000FFFF);
  zbTx.setAddress64(address64);


  // send the payload
  nss.println("   send pay"); 
  xbee.send(zbTx);
//   state1: look for two numbers each < 100
//   found number > 128?
//     state2: observe number
//     if 120,
}

/*
 * fireScript function. Legacy "dumb" receiver scripted ignition and de-activation
 * @param byte scriptNumber the number of the dumb receiver fire script
 */
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
 
      
      // @todo optimize firing scripts:
      
      //   - transmitter sends packet containing entire firing script as arguments to receivers
      //   - receivers ack
      //   - transmitter tells receivers to fire
      
 
      
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
      
      smartClear();
      
      
      
      
      // this code clears DUMB receivers only
//      for(byte receiver = 0; receiver <= receiverCount; receiver ++){
//        for(byte charge = 0; charge <= chargeCount; charge ++) { 
//          setCharge(receiver, charge, 0);  // deactivate charge on receiver
//          nss.print("deact R");
//          nss.print(receiver);
//          nss.print("-");
//          nss.println(charge); 
//     
//        }
//      }
      break;  
    
    default:
      nss.println("firescript # out range");
  }
  notify1(menuOption[0]);
}


/*
 * Displays a little padlock showing that the controls are locked
 */
void menuDisplayLock() {
  if (uiLock) {
    // uiLock is on so clear the entire display before showing the lock
    lcd.clear();  
  }
  
  lcd.setCursor(15, 0);
  lcd.write(2);
  
  int timerNavLock = timer.setTimeout(1000, menuClearLock);
}

/*
 * Clears the padlock icon
 */
void menuClearLock() {
  lcd.setCursor(15, 0);
  lcd.write(' ');
}

/*
 * Function to navigate the menu.
 * @param char navDirection accepts 'u', 'd', 'l', 'r' for up, down, left, right, respectively
 */
void navigateMenu(char navInput) {
  switch (navInput) {
    case 'd':
      if(navLock == 0) {
        // navigation is not locked, move down the menu
        nss.print(" debug: menu size:");
        nss.println((sizeof(firingScriptTitle)/sizeof(firingScriptTitle[0]) - 1));
        nss.print(" debug: activemenu:");
        nss.println(menuActive);
        
        if(menuActive == (sizeof(firingScriptTitle)/sizeof(firingScriptTitle[0]) - 1)) {
          // if we're at the last menu option, loop to the first menu option // ccc
          menuActive = 0;
  
        } else {
          // go to the next menu item
          menuActive ++;
        }
        menuDisplay();
        
      } else {
        menuDisplayLock();
        nss.println("navLock.");
      }
      nss.print("activ menu: ");
      nss.println(menuActive);      
      break;
      
    case 'u':
      // move up the menu
      if(navLock == 0) {
        
        if(menuActive == 0) {
          // if we're at the first menu option, loop to the last menu option
          menuActive = (sizeof(firingScriptTitle)/sizeof(firingScriptTitle[0]) - 1);
          
        } else {
          // go to the previous menu item
          menuActive --;
  
        }      
        menuDisplay();
        
      } else {
        menuDisplayLock();
        nss.println("navLock");
      }
      nss.print("activ menu: ");
      nss.println(menuActive);
      break;
      
    case 'l':
    
      if(navLock == 0) {
        // move left on the menu
        menuDisplay();              // tell program that the menu option needs to be updated
        
      } else {
        menuDisplayLock();
        nss.println("navLock");
      }
      break;
      
    case 'r':
      if(navLock == 0) {
      // move right on the menu
      menuDisplay();                // tell program that the menu option needs to be updated
     
      } else {
        menuDisplayLock();
        nss.println("navLock");
      }
      break;
      
    case 'e':
      // enter on the menu
      //   - new active menu is $(activeMenuOption)

      // if user has been asked to confirm       
      if(bigChoice) {
        // user just confirmed
        // fire the script, unlock controls, and reset bigChoice
        //nss.println("LL firing script");
        //fireScript(menuActive);
        //nss.println("LL unlocking nav");
        notify1(textFiring);
        smartFire(menuActive);
        navLock = 0;
        bigChoice = 0;
        notify1(menuOption[0]);
        notify2(firingScriptTitle[menuActive]);
        
      // user just selected a script
      } else {
        // ask user if they are sure, and lock controls until they answer.
        nss.println("LL Locking nav");
        bigChoice = 1;
        navLock = 1;
        
        notify1(firingScriptTitle[menuActive]);
        notify2("Are you sure?");
      }
      break;
      
    case 'b':
      // back button (*)
      
      // if transmitter is waiting for user to confirm execution
      if (bigChoice == 1) {
        // unlock navigation, we are no longer waiting for confirmation
        navLock = 0;
        bigChoice = 0;
        
        // display script menu
        notify1(menuOption[0]);
        notify2(firingScriptTitle[menuActive]);
      
      } else {
        // do nothing if transmitter was not waiting for confirmation.  
        nss.println("doing nothing");
      break;
      }
  }
}


void keypadCheck(){
  if (!uiLock) {
    // user interface is not locked
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
          break;
        
        case 'B':
          // if B is pressed, do stuff
          break;
          
        case 'C':
          // if C is pressed, do stuff
          smartClear();
          break;
        
        case 'D':
          // if D is pressed, do stuff
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
}


void debugXbee(uint32_t info) {
  nss.println(info, HEX); 
}


char notify1(char *message) {
  lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 0);
  lcd.print(message); 
  nss.println(message);
}

char notify2(char *message) {
  lcd.setCursor(0, 1);
  lcd.print("                "); 
  lcd.setCursor(0, 1);
  lcd.print(message); 
  nss.println(message);
}
