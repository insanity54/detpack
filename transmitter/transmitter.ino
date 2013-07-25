
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
boolean debugMode = 0;
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
  "Gas1",
  "Gas2",
  "Gas3",
  "FOB1",
  "FOB2",
  "FOB3",
  "Olympic1",
  "Olympic2",
  "Olympic3"
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
byte firingScript[][8] = {
  {0,0,128},                         // Gas1
  {0,1,128},                         // Gas2
  {0,2,128},                         // Gas3
  {1,0,128},                         // FOB1
  {1,1,128},                         // FOB2
  {1,2,128},                         // FOB3
  {2,0,128},                         // Olympic1
  {2,1,128},                         // Olympic2
  {2,2,128}                          // Olympic3
};


char* textFiring = "firing!";

//
// INIT
//

uint8_t command[8] = {0};                      // initial payload contents. This is changed by the program.

SoftwareSerial nss(ssRx, ssTx);
XBee xbee = XBee();
XBeeAddress64 address64 = XBeeAddress64(receiverAddress[0][0], receiverAddress[0][1]);  // SH + SL of your receiver radio
RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(address64, chargePinCommand[0], chargePinLow, sizeof(chargePinLow));  // Create a remote AT request with the IR command
RemoteAtCommandResponse remoteAtResponse = RemoteAtCommandResponse();                                             // Create a Remote AT response object
ZBTxRequest zbTx = ZBTxRequest(address64, command, 4);  // address, payload, size
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
SmartPayload smartPayload = SmartPayload();


SimpleTimer timer;                         // timer object
// create timer id that will end display of the splash screen
// create timer that will clear the padlock icon after 1s
int timerNavLock = timer.setTimeout(3000, menuClearLock);


Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// create arrays capable of 99 (100) remotes and 2 (3) charges
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


//void rangeCheck() {
//  // check remote 0
//  //   if remote 0 in range
//  //     display thumbsup
//  
//  lcd.setCursor(0, 0);
//  lcd.print("checking ");
//       
//  memset(command, 0, sizeof(command));  // clear command array (@todo fix this dirty hack)
//  
//  command[0] = 82;  // R
//  command[1] = 65;  // A
//  command[2] = 128; // 128 is our end-of-line delimiters
//  
//  for (int r=0; r < sizeof(receiverCount); r++) {
//    lcd.setCursor(8, 0);
//    lcd.print(r);
//    nss.print("rangecheck:");
//    nss.println(r);
//    
//    address64.setMsb(receiverAddress[r][0]);
//    address64.setLsb(receiverAddress[r][1]);
//    zbTx.setAddress64(address64);
//
//    // send the payload
//    nss.println(" )) range chk"); 
//    xbee.send(zbTx);
    
    
    
    
//    
//    nss.println("set addr");
//    // set address of receiver
//    address64.setMsb(receiverAddress[r][0]);
//    address64.setLsb(receiverAddress[r][1]);
//    remoteAtRequest.setRemoteAddress64(address64);
//        
//    nss.println("set cmd");
//    // set the command 
//    remoteAtRequest.setCommand(rangeCheckCommand);
//    //sremoteAtRequest.setCOmmandValueLength(sizeof(rangeCheckCommand));
//  
//    // send command
//    nss.println("send cmd");
//    xbee.send(remoteAtRequest);  
//    
//    nss.println("read pack");
//    if (xbee.readPacket(5000)) {
//      // got response
//      
//      nss.println("got resp");
//      if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE) {
//        xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);
//        
//        if (remoteAtResponse.isOk()) {
//          lcd.setCursor((10 + r), 0);
//          lcd.print(r);        
//          nss.println("is ok");
//          
//          if (remoteAtResponse.getValueLength() > 0) {
//            nss.print("command val len:");
//            nss.println(remoteAtResponse.getValueLength(), DEC);
//            
//            nss.print("cmd val:");
//            
//            for (int i = 0; i < remoteAtResponse.getValueLength(); i++) {
//              nss.print(remoteAtResponse.getValue()[i], HEX);
//              nss.print(" ");
//            }
//            nss.println();
//          }
//        } else {
//          // not ok (fail)
//          nss.println("not ok");
//          lcd.setCursor((10 + r), 0);
//          lcd.print("-");
//        }
//          
//      } else {
//        nss.print("err:");
//        nss.println(remoteAtResponse.getStatus(), HEX);
//      }
//    } else if (xbee.getResponse().isError()) {
//      nss.print("error:");
//      nss.println(xbee.getResponse().getErrorCode());
//      
//    } else {
//      nss.println("no response");
//    }
//  }
//}


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
        
        //@todo delete
//      case ZB_RX_RESPONSE:
//        // We got a receive packet (0x91)
//        // @todo delete this case. I don't think the transmitter will get this type of packet.
//        nss.println("--> receive pakcet");
//        
//        // fill our zbRxResponse object
//        xbee.getResponse().getZBRxResponse(rx);
//        
//        // Get some ACK & RSSI readings from packet received
//        nss.println(xbee.getResponse().getApiId(), HEX);
//
//        //lcd.setCursor(10, 0);
//        //nss.print("remote address:");
//        //nss.println(rx.getRemoteAddress64().getMsb());
// 
//        // clear the zbRxResponseonse packet
//        // zbRxResponse.clearCommandValue();
//        break;
        
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
 * Set the status of a charge.
 * For dumb receivers only.
 * @param byte remote is the remote number to set
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
      nss.println("deact");
   
      // set the DIO pin HIGH (AT command HEX value 0x5)
      //nss.println("setting DIO pin low");
      remoteAtRequest.setCommandValue(chargePinLow);
      remoteAtRequest.setCommandValueLength(sizeof(chargePinLow));
  
      // send command
      //nss.println("Sending remote at request.");
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
      
      
///*
// * HARD CODED TEST FUNCTION! @todo remove this function with the smartFireActual function
// */
//void smartFireTest(byte scriptNumber) {
//  nss.println("  smartfire called");
//  
//  command[0] = 70;  // F
//  command[1] = 83;  // S   
//  
//  command[2] = 1;   // 
//  command[3] = 0;   //
//  
//  command[4] = 1;   
//  command[5] = 1;   
//  
//  command[6] = 1;
//  command[7] = 2;
//  
//  command[8] = 128;
//  
//  //nss.print("set address");
//  address64.setMsb(receiverAddress[1][0]);
//  address64.setLsb(receiverAddress[1][1]);
//  zbTx.setAddress64(address64);
//  
//  // debug console 
//  nss.println("send 2 rec 1");
//  
//  nss.print("smart cmd: ");
//  for (int ch = 0; ch < sizeof(command); ch++) {
//    nss.print(command[ch]);
//    nss.print(",");
//  }
//  nss.print(command[sizeof(command)]);
//  nss.println(".");
//  
//  xbee.send(zbTx);
//  
//
//}  
      
      
// legacy stuff for leacy stuff. @todo delete me      
//void smartSet(byte derp) {
// // update the object with the correct target address
//    zbTx.setAddress64(address64);
//    // @todo thumbs up icon for ACK
//    
//    if(setStatus) {
//      // we're activating
//      nss.print("smartpay: ");
//      nss.println(smartPayload.requestIgnite(charge, 1, 1));
//      
//      // chunk-of-payload = chunk of smartpayload
//      command[0] = smartPayload.requestIgnite(charge, 1, 0);    
//      command[1] = smartPayload.requestIgnite(charge, 1, 1);    
//      command[2] = smartPayload.requestIgnite(charge, 1, 2);    
//      command[3] = smartPayload.requestIgnite(charge, 1, 3);    
//   
//    } else {
//      // we're de-activating
// 
//      command[0] = smartPayload.requestIgnite(charge, 0, 0);    
//      command[1] = smartPayload.requestIgnite(charge, 0, 1);    
//      command[2] = smartPayload.requestIgnite(charge, 0, 2);    
//      command[3] = smartPayload.requestIgnite(charge, 0, 3);      
//    }
//        
//    // debug console
//    nss.print("command: ");
//    for (int character = 0; character < sizeof(command); character++) {
//      nss.print(command[character]);
//      nss.print(",");
//    }
//    nss.print(command[sizeof(command)]);
//    nss.println(".");
//    
//    // send the payload    
//    xbee.send(zbTx);
//      
//    break;  // out of smart receiver case
//
//}  
      
      
      
/*
 * Sends fire script commands to smart receivers.
 * 
 * @param int mode the two character mode ID for the desired command ex: 7083 (FS)
 * @param scriptNumber
 */      
//void smartCommand(int mode = 7083, byte scriptNumber = 0) {
//  
//  command[0] = mode / 100;  // ex: 7083 #> 70
//  command[1] = mode % 100;  // ex: 7083 #> 83
//  
//  
//}  
            
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



void smartFire(byte scriptNumber) {
  // set the first two bytes of the command string
  // @todo make this set dynamically so it works for commands other than 'FS' (?)
  
  command[0] = 70;  // F
  command[1] = 83;  // S
  
  
// STATE ENGINE
  nss.println("fire script");
// if(menuActive == (sizeof(firingScriptTitle)/sizeof(firingScriptTitle[0]) - 1)) {
  
//  nss.print("size compoz:");
//  nss.println((sizeof(firingScript)/sizeof(firingScript[0])));
//  
//  nss.print("size overall");
//  nss.println(sizeof(firingScript));
//  
//  nss.print("size jusone:");
//  nss.println(sizeof(firingScript[0]));

  for (int character = 0; character < sizeof(firingScript[scriptNumber]); character++) {
    // sizeof(firingScript[0]) will equal x: firingScript[][x]
    // iterate through characters of firingScripts

        
    
    if (firingScript[scriptNumber][character] == 128) {
      // if character is 128 stop checking this script and go to next script
      // 128 is an EOL delimeter
      
      // add the delimeter to the command payload
      // offset by +2 because the first two of command payload are 'F' and 'S'
      command[character+2] = 128;
      break;  // stop adding characters because 128 indicated the end of the script
      
    } else {
      // character is not 128
      // add the number to the command payload
      // offset by +2 because the first two of command payload are 'F' and 'S'
      
      nss.print(" character");
      nss.print(character);
      nss.print(": ");
      nss.println(firingScript[scriptNumber][character]);
      
      // @todo this line causes a crash when either writing command[24] or accessing firingScript[scriptNumber][24]
      //       worked around for now by preventing it from writing to [24]
      if (character < 22) {
        command[character+2] = firingScript[scriptNumber][character];
        
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


///*
// * Asks the user if they are sure they want to execute the action.
// * Accepts a fireScript number as a parameter
// * if parameter -1 is given, function will
// * return 1 if user has been asked if they're sure
// * return 0 if user has not been asked if they're sure
// * 
// * @param scriptNum byte
// * @return boolean
// */
//boolean menuDecision(char scriptNum) {
//  
//  if(scriptNum > 0) {
//    // ask user if they're sure
//    notify1(firingScriptTitle[scriptNum]);
//    notify2("Are you sure?");
//    
//    nss.println("LL ARE YOU SURE?");
//    
//    // set the global variable to show that the user has a big choice to make
//    // (during this time, the user shouldn't be able to navigate, only choose yes or no)
//    bigChoice = 1;
//    
//  } else {
//    nss.print("LL just checking:");
//    // return a value so the code calling this function knows if
//    // if the user has a big choice to make
//    
//    if(bigChoice == 1) {
//      // user has to make a choice
//      nss.println("user has to decide");
//      return 1;
//      
//    } else {
//      // user does not have to make a choice
//      nss.println("user not have choice");
//      return 0;
//    }
//  }
//}

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
          //rangeCheck();
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
