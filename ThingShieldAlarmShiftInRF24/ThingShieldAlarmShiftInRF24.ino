//*****************************************************************************
/// Arduino SmartThings Alarm
/// https://github.com/coolkev/smartthings-alarm
///
/*  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 */
//*****************************************************************************

#define USE_SMARTTHINGS

#if defined USE_SMARTTHINGS
  #include <SoftwareSerial.h>   //TODO need to set due to some weird wire language linker, should we absorb this whole library into smartthings
  #include <SmartThings.h>
#endif 

//#define USE_RF24_MESH

#if defined USE_RF24_MESH
  #include "RF24.h"
  #include "RF24Network.h"
  #include "RF24Mesh.h"
  #include <SPI.h>
  
  //Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
  #include <EEPROM.h>
#endif 



/* Type of Shift Register - uncomment only one of the following lines depending on the type of shift register being used */
//#define SHIFT_REGISTER_CD4021B
#define SHIFT_REGISTER_74HC165


/* How many shift register chips are daisy-chained. */
#define NUMBER_OF_SHIFT_CHIPS   1

/* Width of data (how many ext lines). */
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch. */
#define PULSE_WIDTH_USEC   5

int latchPin = 7; //Calin = 5
int dataPin = 5; //Calin = 7
int clockPin = 4;

int pinValues = -1;

bool isDebugEnabled = true;    // enable or disable debug in this example

int lastHeartbeat = 0;
long heartbeatMs = 10000; // 1 min

int doorbellPin = A5;
String doorbellPushedCommand = "w 12 1";
int doorbellState = HIGH;

boolean openstate = true; // false if using pull-down resistors, true if using pull-up resistors


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

#if defined USE_SMARTTHINGS
  
  #define PIN_THING_RX    3
  #define PIN_THING_TX    2
  SmartThingsCallout_t messageCallout;    // call out function forward decalaration
  SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout, "GenericShield",false);  // constructor
#endif

#if defined USE_RF24_MESH
  /***** Configure the chosen CE,CS pins *****/
  RF24 radio(8,10);
  RF24Network network(radio);
  RF24Mesh mesh(radio,network);
#endif





void setupWiredZones() {
 
 
      
    //define pin modes
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT); 
    pinMode(dataPin, INPUT);

  
    pinMode(doorbellPin, INPUT);
    digitalWrite(doorbellPin, HIGH);
}

#if defined USE_RF24_MESH
void setupWirelessMesh() {
 

  Serial.println("Starting mesh network as Master Node");
  
    // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());
  // Connect to the mesh
  mesh.begin();

  
}

#endif
void sendHeartbeat() {
  
  if (((millis()) - lastHeartbeat*heartbeatMs) > heartbeatMs) {
      
    //Serial.print("(millis()/heartbeatMs) - lastHeartbeat=");
    //Serial.println((millis()/heartbeatMs) - lastHeartbeat);
    
    
    lastHeartbeat = millis()/heartbeatMs;
    
    
    String statusString= "hb ";
    statusString += lastHeartbeat;
   
    
    smartthingsend(statusString);
  }
  
  
}

void updateWiredZones() {

  doorbellPin = A5;
  
  int newDoorbellState = digitalRead(doorbellPin);
  if (newDoorbellState == LOW && newDoorbellState != doorbellState)
  {
      Serial.print("doorbell pressed");
      
      smartthingsend(doorbellPushedCommand);
  }

    /* Read in and display the pin states at startup.
    */
    int newPinValues = shiftIn(latchPin, dataPin, clockPin);

    if (newPinValues==pinValues)  {
      //nothing has changed
       return; 
    }

    Serial.print("newPinValues=");
    Serial.println(newPinValues);
    
    for (int x = 0; x<DATA_WIDTH; x++) {
      
      boolean prevReading = (pinValues >> x) & 1;
      boolean newReading = (newPinValues >> x) & 1;
            
      if (pinValues==-1 || newReading!=prevReading)
      {
                  
           
        String statusString= "w ";
       
        statusString += x+1;
        
        statusString += newReading==openstate ? " 0" : " 1";
        
       
        smartthingsend(statusString);
          
        
      }
    
    }
    
    pinValues = newPinValues;
  

}

#if defined USE_RF24_MESH
void updateWirelessZones() {
 
 
  // Call mesh.update to keep the network updated
  mesh.update();
  
  // In addition, keep the 'DHCP service' running on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
  
  
  // Check for incoming data from the sensors
  if(network.available()){
    RF24NetworkHeader header;        // If so, grab it and print it out
    byte reading;
    network.read(header,&reading,sizeof(reading));
    
    String statusString= "r ";
   
    char device = header.type;
    statusString += device;
    statusString += " ";
    statusString += reading;
    
    smartthingsend(statusString);
  }
   
  
  
}

#endif

void smartthingsend(String message) {
 
     
    Serial.print(message);
  
long start = millis();

#if defined USE_SMARTTHINGS      
    smartthing.send(message);
#endif

int duration = millis()-start;  
  
  Serial.print(" (took ");
  Serial.print(duration);
  Serial.println(" ms)");  
  
}


//#if defined USE_SERIAL_ZONES
//  void updateSerialZones() {
//    
////    if (!mySerial.isListening()) {
////      Serial.println("serial not listening - calling listen()");
////      mySerial.listen();
////    }
//    
//    if (!Serial.available()) {
//      //mySerial.end();
//      return;
//    }  
//    Serial.println("Receiving serial data");
//  
//    String content = "";
//    char character;
//    
//     while(Serial.available()) {
//      character = Serial.read();
//      
//      if (character==10 || character==13)
//        break;
//      
//      // characters out of range will cause us to just throw out this whole reading
//      if (character!=13 && (character>=127 || character<32))
//      {  
//         Serial.println("Character out of range - ignoring serial data");
//         Serial.println(character,DEC);
//         while(Serial.available()) {
//            Serial.read(); 
//         }
//         return; 
//      }
//      content.concat(character);
//    }
//
//    //mySerial.end();
//      
//  if (content != "") {
//    Serial.println(content);
//    
//    #if defined USE_SMARTTHINGS
//      smartthing.send(content);
//    #endif
//    delay(100);
//  }
//    
//    
// }
//  
//#endif

void messageCallout(String message)
{
  // if debug is enabled print out the received message
  if (isDebugEnabled && message!="")
  {
    Serial.print("Received message: '");
    Serial.print(message);
    Serial.println("' ");
  }

  // if message contents equals to 'on' then call on() function
  // else if message contents equals to 'off' then call off() function
  if (message.equals("on"))
  {
   // on();
  }
  else if (message.equals("off"))
  {
   // off();
  }
  else if (message.equals("hello"))
  {
//    hello();
  }
}



#if defined SHIFT_REGISTER_CD4021B
int shiftIn(int myLatchPin, int myDataPin, int myClockPin) { 
  int pinState;
  int result = 0;


    digitalWrite(myLatchPin,HIGH);
    delayMicroseconds(20);
    digitalWrite(myLatchPin,LOW);


  for (int i=DATA_WIDTH-1; i>=0; i--)
  {
    digitalWrite(myClockPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);

    pinState = digitalRead(myDataPin);
    result |= (pinState << i);

    digitalWrite(myClockPin, HIGH);

  }
  
  return result;
}
#endif
#if defined SHIFT_REGISTER_74HC165
int shiftIn(int myLatchPin, int myDataPin, int myClockPin) { 

  int pinState;
  int result = 0;
  

    digitalWrite(myLatchPin, LOW);
    delayMicroseconds(20);
    digitalWrite(myLatchPin, HIGH);


  for (int i=0; i< DATA_WIDTH; i++)
  {
    
    digitalWrite(myClockPin, LOW);
    pinState = digitalRead(myDataPin);
    result |= (pinState << (DATA_WIDTH-1-i));

    digitalWrite(myClockPin, HIGH);
    delayMicroseconds(PULSE_WIDTH_USEC);

  }
  
  return result;
}
#endif


void loop()
{
  
  
#if defined USE_SMARTTHINGS
  smartthing.run();
#endif

  updateWiredZones();
  
  sendHeartbeat();

#if defined USE_RF24_MESH
  updateWirelessZones();
#endif  

  if (stringComplete) {
    smartthingsend(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  
}

void setup()
{
  // setup default state of global variables


  Serial.begin(9600);         // setup serial with a baud rate of 9600
  Serial.println("setup..");  // print out 'setup..' on start

  #if defined USE_RF24_MESH
  
    setupWirelessMesh();
    
  #endif

  setupWiredZones();
  
  inputString.reserve(200);
}


void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
    else if (inChar<127 && inChar>=32) {
      inputString += inChar;
    }
  }
}
