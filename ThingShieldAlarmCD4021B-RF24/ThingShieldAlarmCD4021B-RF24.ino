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

#include <SoftwareSerial.h>   //TODO need to set due to some weird wire language linker, should we absorb this whole library into smartthings
#include <SmartThings.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>

//Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
#include <EEPROM.h>

#define PIN_THING_RX    3
#define PIN_THING_TX    2

/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   2

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   2

int latchPin = 5;
int dataPin = 7;
int clockPin = 4;

int pinValues = -1;

bool isDebugEnabled = true;    // enable or disable debug in this example



SmartThingsCallout_t messageCallout;    // call out function forward decalaration
SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout, "GenericShield",false);  // constructor

/***** Configure the chosen CE,CS pins *****/
RF24 radio(8,10);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

long lastHeartbeat = 0;
long heartbeatMs = 60000; // 1 min

boolean openstate = false; // false if using pull-down resistors, true if using pull-up resistors

void setup()
{
  // setup default state of global variables

  if (isDebugEnabled)
  { // setup debug serial port
    Serial.begin(57600);         // setup serial with a baud rate of 9600
    Serial.println("setup..");  // print out 'setup..' on start
  }  
  
  //setupWirelessMesh();

  setupWiredZones();
  
}



void setupWiredZones() {
 
 
    Serial.println("Checking initial zone states");
      
    //define pin modes
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT); 
    pinMode(dataPin, INPUT);


  
  
}


void setupWirelessMesh() {
 
  
  Serial.println("Starting mesh network as Master Node");
  
    // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());
  // Connect to the mesh
  mesh.begin();

  
}

void loop()
{
  smartthing.run();

  sendHeartbeat();
  
  updateWiredZones();
  
  //updateWirelessZones();
  
  
}

void sendHeartbeat() {
  
  if (lastHeartbeat==0 || (millis() - lastHeartbeat) > heartbeatMs) {
    lastHeartbeat = millis();
    
    
    String statusString= "heartbeat ";
    statusString += lastHeartbeat;
    
    Serial.print("sending ");
    Serial.println(statusString);
    
     smartthing.send(statusString);

  }
  
  
}

void updateWiredZones() {

  
    //Pulse the latch pin:
    //set it to 1 to collect parallel data
    digitalWrite(latchPin,HIGH);
    //set it to 1 to collect parallel data, wait
    delayMicroseconds(20);
    //set it to 0 to transmit data serially  
    digitalWrite(latchPin,LOW);


    /* Read in and display the pin states at startup.
    */
    int newPinValues = shiftIn(dataPin, clockPin);

    
    if (newPinValues==pinValues)  {
      //nothing has changed
       return; 
    }

    for (int x = 0; x<DATA_WIDTH; x++) {
      
      boolean prevReading = (pinValues >> x) & 1;
      boolean newReading = (newPinValues >> x) & 1;
            
      if (newReading!=prevReading)
      {
                  
           
        String statusString= "wired ";
       
        statusString += x+1;
        
        statusString += newReading==openstate ? " open" : " closed";
        
       
        Serial.println(statusString);
        smartthing.send(statusString);
        
           
      }
    
    }
    
    pinValues = newPinValues;
  

}


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
    
    String statusString= "wireless ";
   
    char device = header.type;
    statusString += device;
    statusString += " ";
    statusString += reading;
    
    Serial.println(statusString);
    smartthing.send(statusString);
    
  }
   
  
  
}

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



/* This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
//BYTES_VAL_T read_shift_regs()
//{
//    long bitVal;
//    BYTES_VAL_T bytesVal = 0;
//
//    /* Trigger a parallel Load to latch the state of the data lines,
//    */
//    //digitalWrite(clockEnablePin, HIGH);
//    digitalWrite(ploadPin, LOW);
//    delayMicroseconds(PULSE_WIDTH_USEC);
//    digitalWrite(ploadPin, HIGH);
//    //digitalWrite(clockEnablePin, LOW);
//
//    
//    /* Loop to read each bit value from the serial out line
//     * of the SN74HC165N.
//    */
//    for(int i = 0; i < DATA_WIDTH; i++)
//    {
//        bitVal = digitalRead(dataPin);
//
//        /* Set the corresponding bit in bytesVal.
//        */
//        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));
//
//        /* Pulse the Clock (rising edge shifts the next bit).
//        */
//        digitalWrite(clockPin, HIGH);
//        delayMicroseconds(PULSE_WIDTH_USEC);
//        digitalWrite(clockPin, LOW);
//    }
//
//    return(bytesVal);
//}



int shiftIn(int myDataPin, int myClockPin) { 
  //int i;
  //int temp = 0;
  int pinState;
  int result = 0;

  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, INPUT);

//we will be holding the clock pin high 8 times (0,..,7) at the
//end of each time through the for loop

//at the begining of each loop when we set the clock low, it will
//be doing the necessary low to high drop to cause the shift
//register's DataPin to change state based on the value
//of the next bit in its serial information flow.
//The register transmits the information about the pins from pin 7 to pin 0
//so that is why our function counts down
  for (int i=DATA_WIDTH-1; i>=0; i--)
  {
    digitalWrite(myClockPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);

    pinState = digitalRead(myDataPin);
    result |= (pinState << i);

    digitalWrite(myClockPin, HIGH);

  }
  //debuging print statements whitespace
  //Serial.println();
  //Serial.println(myDataIn, BIN);
  return result;
}
