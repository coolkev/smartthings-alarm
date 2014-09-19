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


#define PIN_THING_RX    3
#define PIN_THING_TX    2

#define ledPin  13

#define ZONE_COUNT  8
int zonePins[] = {0,5,7,8,9,10,11,12};
int zoneStates[ZONE_COUNT];
long zoneTimes[ZONE_COUNT];         // the last time the output pin was toggled

long debouncedelay = 10; 

bool isDebugEnabled = true;    // enable or disable debug in this example
int stateLED;           // state to track last set value of LED


SmartThingsCallout_t messageCallout;    // call out function forward decalaration
SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout, "GenericShield",false);  // constructor


void setup()
{
  // setup default state of global variables
  isDebugEnabled = true;
  stateLED = 0;                 // matches state of hardware pin set below
  
  // setup hardware pins 
  pinMode(ledPin, OUTPUT);     // define PIN_LED as an output
  //digitalWrite(ledPin, LOW);   // set value to LOW (off) to match stateLED=0
  digitalWrite(ledPin, HIGH);

  if (isDebugEnabled)
  { // setup debug serial port
    Serial.begin(9600);         // setup serial with a baud rate of 9600
    Serial.println("setup..");  // print out 'setup..' on start
  }
  
    Serial.println("Checking initial zone states");
    
  for (int x = 0; x<ZONE_COUNT; x++) {
    pinMode(zonePins[x],INPUT_PULLUP);
    //zoneStates[x] = LOW;
    //zoneTimes[x] = 0;
    
    int reading = digitalRead(zonePins[x]);
    
    int zoneNum = x+1;


    String statusString= "zonestatus ";
   
    statusString += zoneNum;
    
    statusString += reading == LOW ? " closed" : " open";
    
 
    Serial.println(statusString);
    smartthing.send(statusString);
    
    zoneStates[x] = reading;
                 
    zoneTimes[x] = millis();    
     
    
  }
  
}



void loop()
{
  smartthing.run();

  
  for (int x = 0; x<ZONE_COUNT; x++) {
  
    int reading = digitalRead(zonePins[x]);
    
    int zoneNum = x+1;
    if (reading!=zoneStates[x])
    {
       
         if ((millis() - zoneTimes[x]) > debouncedelay) {
                      
                            
            String statusString= "zonestatus ";
           
            statusString += zoneNum;
            
            statusString += reading == LOW ? " closed" : " open";
            
            Serial.println(statusString);
            smartthing.send(statusString);
    
            zoneStates[x] = reading;
            
        }
        
        zoneTimes[x] = millis();    
     
     }
    
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
