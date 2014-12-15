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

#include <SoftEasyTransfer.h>

SoftwareSerial mySerial(A3, A4); // RX, TX

SoftEasyTransfer ET; 

struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  byte sensor;
  int reading;
};

//give a name to the group of data
SEND_DATA_STRUCTURE mydata;

//#define USE_RF24_MESH

//#if defined USE_RF24_MESH
//  #include "RF24.h"
//  #include "RF24Network.h"
//  #include "RF24Mesh.h"
//  #include <SPI.h>
//  
//  //Include eeprom.h for AVR (Uno, Nano) etc. except ATTiny
//  #include <EEPROM.h>
//#endif 



/* Type of Shift Register - uncomment only one of the following lines depending on the type of shift register being used */
//#define SHIFT_REGISTER_CD4021B
#define SHIFT_REGISTER_74HC165


/* How many shift register chips are daisy-chained. */
#define NUMBER_OF_SHIFT_CHIPS   1

/* Width of data (how many ext lines). */
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch. */
#define PULSE_WIDTH_USEC   2

int latchPin = 7;
int dataPin = 5;
int clockPin = 4;

int pinValues = -1;

bool isDebugEnabled = true;    // enable or disable debug in this example

int lastHeartbeat = 0;
long heartbeatMs = 60000; // 1 min

#define DOORBELL_NOTIFY
#if defined DOORBELL_NOTIFY
  int doorbellPin = A5;
  String doorbellPushedCommand = "w 11 1";
  int doorbellState = HIGH;
#endif

//#define TEMP_SENSORS
#if defined TEMP_SENSORS
  //#include <OneWire.h>
//#endif
//#if defined TEMP_SENSORS
  int tempDeviceStart = 12;
  int temppin = 8;
  OneWire ds(temppin);  // on pin 10 (a 4.7K resistor is necessary)
  float temperature[10];
  long temperatureAt = 0;
  long temperatureInterval = 10000; // only check temp every 10 secs


#endif

boolean openstate = true; // false if using pull-down resistors, true if using pull-up resistors


//String inputString = "";         // a string to hold incoming data
//boolean stringComplete = false;  // whether the string is complete

#if defined USE_SMARTTHINGS
  
  #define PIN_THING_RX    3
  #define PIN_THING_TX    2
  SmartThingsCallout_t messageCallout;    // call out function forward decalaration
  SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout, "GenericShield",false);  // constructor
#endif

#if defined USE_RF24_MESH
  /***** Configure the chosen CE,CS pins *****/
  RF24 radio(7,8);
  RF24Network network(radio);
  RF24Mesh mesh(radio,network);
#endif





void setupWiredZones() {
 
 
      
    //define pin modes
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT); 
    pinMode(dataPin, INPUT);

    #if defined DOORBELL_NOTIFY
    pinMode(doorbellPin, INPUT);
    digitalWrite(doorbellPin, HIGH);
    #endif
}

#if defined USE_RF24_MESH
void setupWirelessMesh() {
 

  Serial.println("Starting mesh network as Master Node");
  
    // Set the nodeID to 0 for the master node
  mesh.setNodeID(0);
  Serial.println(mesh.getNodeID());
  // Connect to the mesh
  mesh.begin();

  radio.printDetails();
  
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

  
  #if defined DOORBELL_NOTIFY
  int newDoorbellState = digitalRead(doorbellPin);
  if (newDoorbellState == LOW && newDoorbellState != doorbellState)
  {
      Serial.print("doorbell pressed");
      
      smartthingsend(doorbellPushedCommand);
  }
  #endif
  
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

//  if (stringComplete) {
//    smartthingsend(inputString);
//    // clear the string:
//    inputString = "";
//    stringComplete = false;
//  }
  
  
  
  
  #if defined TEMP_SENSORS
    readTemperature(false);
   #endif
   
   
   //check and see if a data packet has come in. 
  if(ET.receiveData()){
    
    String statusString = "w ";
    statusString += mydata.sensor;
    statusString += " ";
    statusString += mydata.reading;

    smartthingsend(statusString);
    
  }
  
  
}

void setup()
{
  // setup default state of global variables


  Serial.begin(4800);         // setup serial with a baud rate of 9600
  Serial.println("setup..");  // print out 'setup..' on start

  mySerial.begin(9600);
  //start the library, pass in the data details and the name of the serial port.
  ET.begin(details(mydata), &mySerial);
  
  #if defined USE_RF24_MESH
  
    setupWirelessMesh();
    
  #endif

  setupWiredZones();
  
//  inputString.reserve(10);
  
  
  #if defined TEMP_SENSORS
    readTemperature(true);
   #endif
}


//void serialEvent() {
//  while (Serial.available()) {
//    // get the new byte:
//    char inChar = (char)Serial.read(); 
//    // add it to the inputString:
//    
//    // if the incoming character is a newline, set a flag
//    // so the main loop can do something about it:
//    if (inChar == '\n') {
//      stringComplete = true;
//    } 
//    else if (inChar<127 && inChar>=32) {
//      inputString += inChar;
//    }
//  }
//}



#if defined TEMP_SENSORS
void readTemperature(boolean printChipInfo) {
 
  
  if (temperatureAt+temperatureInterval > millis()) {
    
    return;
    
  }

  temperatureAt = millis();

  //Serial.println("checking Temp");
  
  ds.reset_search();
  int sensorNum = 0;
  
  byte addr[8];
    
  while (ds.search(addr)) {
    
    Serial.print("ROM =");
    for(int i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
    }
    
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    float celsius, fahrenheit;
    
    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return;
    }
    //Serial.println();
   String chipInfo;
    // the first ROM byte indicates which chip
    switch (addr[0]) {
      case 0x10:
        chipInfo = "  Chip = DS18S20";  // or old DS1820
        type_s = 1;
        break;
      case 0x28:
        chipInfo = "  Chip = DS18B20";
        type_s = 0;
        break;
      case 0x22:
        chipInfo = "  Chip = DS1822";
        type_s = 0;
        break;
      default:
        chipInfo = "Device is not a DS18x20 family device.";
        return;
    } 
  
    //if (printChipInfo) {
      
        Serial.println(chipInfo);
        
    //}
    //if (temperatureAt==0) {
        
      ds.reset();
      ds.select(addr);
      ds.write(0x44, 0);        // start conversion, with parasite power on at the end
      
      delay(1000);
      
      //delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    //}
    
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad
  
//    Serial.print("  Data = ");
//    Serial.print(present, HEX);
//    Serial.print(" ");
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
      //Serial.print(data[i], HEX);
      //Serial.print(" ");
    }
    
      
      
//    Serial.print(" CRC=");
//    Serial.print(OneWire::crc8(data, 8), HEX);
//    Serial.println();
//  
    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } else {
      byte cfg = (data[4] & 0x60);
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    fahrenheit = celsius * 1.8 + 32.0;
    //Serial.print("Temperature = ");
    //Serial.print(celsius);
    //Serial.print(" Celsius, ");
    //Serial.print(fahrenheit);
    //Serial.println(" Fahrenheit");



    if (abs(temperature[sensorNum]-fahrenheit)>=0.5) {
  
        temperature[sensorNum] = fahrenheit;
       
         String statusString = "w ";
        statusString += sensorNum+tempDeviceStart;
        statusString += " ";
        statusString += fahrenheit;
        
        smartthingsend(statusString);
        
       Serial.print("w ");
       Serial.print(sensorNum+tempDeviceStart);
       Serial.print(" ");
       Serial.println(fahrenheit);
       
       //mySerial.println("w 9 1");      
    }
  
    sensorNum++;
  
  
  }


}
#endif

