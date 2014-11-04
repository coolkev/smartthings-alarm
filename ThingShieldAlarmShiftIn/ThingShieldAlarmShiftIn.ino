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

/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   1

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   1

/* You will need to change the "int" to "long" If the
 * NUMBER_OF_SHIFT_CHIPS is higher than 2.
*/
#define BYTES_VAL_T unsigned int

int ploadPin        = 3; //8;  // Connects to Parallel load pin the 165
//int clockEnablePin  = 9;  // Connects to Clock Enable pin the 165
int dataPin         = 2; //11; // Connects to the Q7 pin the 165
int clockPin        = 4; //12; // Connects to the Clock pin the 165

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;



#define ZONE_COUNT  8

boolean zoneStates[ZONE_COUNT];
long zoneTimes[ZONE_COUNT];         // the last time the output pin was toggled

long debouncedelay = 10; 

bool isDebugEnabled = true;    // enable or disable debug in this example


SmartThingsCallout_t messageCallout;    // call out function forward decalaration
SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout, "GenericShield",false);  // constructor


void setup()
{
  // setup default state of global variables
  isDebugEnabled = true;

  if (isDebugEnabled)
  { // setup debug serial port
    Serial.begin(57600);         // setup serial with a baud rate of 9600
    Serial.println("setup..");  // print out 'setup..' on start
  }
  

    Serial.println("Checking initial zone states");
    
      pinMode(ploadPin, OUTPUT);
   // pinMode(clockEnablePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, INPUT);

    digitalWrite(clockPin, LOW);
    digitalWrite(ploadPin, HIGH);

    /* Read in and display the pin states at startup.
    */
    pinValues = read_shift_regs();

    oldPinValues = pinValues;
    
  for (int x = 0; x<ZONE_COUNT; x++) {
    
    int zoneNum = x+1;
    
    boolean reading = (pinValues >> x) & 1;

    String statusString= "zonestatus ";
   
    statusString += zoneNum;
    
    statusString += reading ? " open" : " closed";
    
 
    Serial.println(statusString);
    smartthing.send(statusString);
    
    zoneStates[x] = reading;
                 
    zoneTimes[x] = millis();    
     
    
  }
  
}



void loop()
{
  smartthing.run();

  pinValues = read_shift_regs();
  
  for (int x = 0; x<ZONE_COUNT; x++) {
  
    boolean reading = (pinValues >> x) & 1;
    
    int zoneNum = x+1;
    if (reading!=zoneStates[x])
    {
       
         if ((millis() - zoneTimes[x]) > debouncedelay) {
                      
                            
            String statusString= "zonestatus ";
           
            statusString += zoneNum;
            
            statusString += reading ? " open" : " closed";
            
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



/* This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
    long bitVal;
    BYTES_VAL_T bytesVal = 0;

    /* Trigger a parallel Load to latch the state of the data lines,
    */
    //digitalWrite(clockEnablePin, HIGH);
    digitalWrite(ploadPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(ploadPin, HIGH);
    //digitalWrite(clockEnablePin, LOW);

    
    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(dataPin);

        /* Set the corresponding bit in bytesVal.
        */
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockPin, LOW);
    }

    return(bytesVal);
}



