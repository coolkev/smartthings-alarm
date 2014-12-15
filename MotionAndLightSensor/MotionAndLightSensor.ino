#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 5
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address


int lightpin = A0;
int motionpin = 4;
int ledpin = 2;
int temppin = 5;
int tempground = 7;
int tempDeviceStart = 12;

boolean lastMotion = false;
long lastMotionAt = 0;

int lightvalue = 0;
long lightvalueAt = 0;
float temperature[10];
long temperatureAt = -10000;
long temperatureInterval = 10000; // only check temp every 30 secs

long ledOnAt = 0;
boolean ledOn = false;
long lastStatusPrint = 0;
long statusPrintTiming = 1000;

OneWire ds(temppin);  // on pin 10 (a 4.7K resistor is necessary)

#include <SoftwareSerial.h>
#include <SoftEasyTransfer.h>

SoftwareSerial mySerial(A3, A5); // RX, TX

SoftEasyTransfer ET; 

struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  byte sensor;
  int reading;
};

//give a name to the group of data
SEND_DATA_STRUCTURE mydata;


void setup()
{
  
  Serial.begin(57600);      
  Serial.println("setup.."); 
  
  mySerial.begin(9600);
  
  ET.begin(details(mydata), &mySerial);
  
  pinMode(lightpin, INPUT);
  pinMode(motionpin, INPUT);
  
  
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);
  
  pinMode(tempground, OUTPUT);
  digitalWrite(tempground, LOW);
    
  initTemperatures();
  
  
}


void loop() {
  
  
  
  int lightvalueNew = int(analogRead(lightpin)/1024.0*100)*10;
  
  int lightChange = abs(lightvalueNew-lightvalue);
  //only if changed by >= 10
  if (lightChange>=10) {

     
     //and at most every 10 seconds or > 100 change
     if (lightChange>=100 || (millis() - lightvalueAt) >= 10000) 
     {
       lightvalueAt = millis();
       lightvalue = lightvalueNew;
       
       Serial.print("w 10 ");
       Serial.println(lightvalueNew);
       
       mydata.sensor = 10;
       mydata.reading = lightvalueNew;

       //send the data
       ET.sendData();
       
     }
  }
  

    
  
  boolean motionvalue = digitalRead(motionpin) == HIGH;
  
  if (motionvalue && !lastMotion) {

     lastMotionAt = millis();

     digitalWrite(ledpin, HIGH);
     ledOn = true;
     ledOnAt = millis();
   
     Serial.println("w 9 0");
     //mySerial.println("w 9 0");

     mydata.sensor = 9;
     mydata.reading = 0;

     //send the data
     ET.sendData();
      
  }
  else if (lastMotion && !motionvalue) {
    
     Serial.println("w 9 1");    
     //mySerial.println("w 9 1");    
     digitalWrite(ledpin, LOW);
     
     
     mydata.sensor = 9;
     mydata.reading = 1;

     //send the data
     ET.sendData();
  }
  
  lastMotion = motionvalue;
  

  if (temperatureAt+temperatureInterval <= millis()) {
      
    temperatureAt = millis();
    
    readTemperatures();
    
  }

  //readTemperature(false);
  //printStatus();
  
  

}



void initTemperatures() {
  
  
  // Start up the library
  sensors.begin();
  
  // Grab a count of devices on the wire
  int numberOfDevices = sensors.getDeviceCount();
  
  // locate devices on the bus
  Serial.print("Locating devices...");
  
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
	{
		Serial.print("Found device ");
		Serial.print(i, DEC);
		//Serial.print(" with address: ");
//		printAddress(tempDeviceAddress);
		Serial.println();
		
		Serial.print("Setting resolution to ");
		Serial.println(TEMPERATURE_PRECISION, DEC);
		
		// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
		sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
		
		 Serial.print("Resolution actually set to: ");
		Serial.print(sensors.getResolution(tempDeviceAddress), DEC); 
		Serial.println();
	}else{
		Serial.print("Found ghost device at ");
		Serial.print(i, DEC);
		Serial.print(" but could not detect address. Check power and cabling");
	}
  }
  
}


void readTemperatures() {
  
  
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println("DONE");
  
  int numberOfDevices = sensors.getDeviceCount();  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++)
  {
    
    float fahrenheit = sensors.getTempFByIndex(i);
    // Search the wire for address
    if(fahrenheit != DEVICE_DISCONNECTED_F)
    {
      
      if (abs(temperature[i]-fahrenheit)>=1) {
    
          temperature[i] = fahrenheit;
       
         Serial.print("w ");
         Serial.print(i+tempDeviceStart);
         Serial.print(" ");
         Serial.println(fahrenheit);
         
         
         mydata.sensor = i+tempDeviceStart;
         mydata.reading = int(fahrenheit);
    
         //send the data
         ET.sendData();
             
      }
    } 
    //else ghost device! Check your power requirements and cabling
	
  }
  
  
}  


void printStatus() {
 
 
  if (lastStatusPrint+statusPrintTiming < millis()) {
    
    
    Serial.print("temps: ");
    
    for (int x=0;x<10;x++) {
     
     if (temperature[x]>0) {

       Serial.print(temperature[x]);
       Serial.print("/");
     } 
      
    }
    
    
        
    Serial.print("\t current light level: ");
    Serial.print(lightvalue);
    
    
    Serial.print("\t motion last detected: ");
    if (lastMotionAt==0) {
      Serial.print("never   ");      
    }
    else {
      Serial.print((millis()-lastMotionAt)/(long)1000);
      Serial.print("s ago");
    }
    
    Serial.print("\t led state: ");
    Serial.print(ledOn ? "on" : "off");

    Serial.println();
    
    lastStatusPrint = millis();
    
  } 
  
}
