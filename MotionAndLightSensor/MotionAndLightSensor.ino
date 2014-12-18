#include <OneWire.h>
#include <DallasTemperature.h>
#include <SimpleTimer.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 5
#define TEMPERATURE_PRECISION 10

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

SimpleTimer timer;

long lightLevelInterval = 1000;
long temperatureInterval = 10000; // only check temp every 30 secs

int lightvals[10];
int lightpin = A0;
int motionpin = 4;
int ledpin = 2;
int tempground = 7;
int tempDeviceStart = 12;

boolean lastMotion = false;
long lastMotionAt = 0;

int lightvalue = 0;
float temperature[10];


long ledOnAt = 0;
boolean ledOn = false;
long lastStatusPrint = 0;
long statusPrintTiming = 1000;

#include <SoftwareSerial.h>
#include <SoftEasyTransfer.h>

SoftwareSerial mySerial(A3, A5); // RX, TX

SoftEasyTransfer ET; 

struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  byte sensor;
  int reading;
  boolean readingNegative;
//  char message[50];
};

//give a name to the group of data
SEND_DATA_STRUCTURE mydata;


void setup()
{
  
  Serial.begin(57600);      
  Serial.println("setup.."); 
  
  mySerial.begin(2400);
  
  ET.begin(details(mydata), &mySerial);
  
  pinMode(lightpin, INPUT);
  pinMode(motionpin, INPUT);
  
  
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);
  
  initTemperatures();
  
  
  timer.setInterval(lightLevelInterval, checkLightLevelBig);
  timer.setInterval(lightLevelInterval*60, checkLightLevelSmall);
    
  timer.setInterval(temperatureInterval, checkTemperatures);
  
  String msg = "Garage sensor started ";
  
  msg += sensors.getDeviceCount();
  msg += " temp sensors";
  
  Serial.println(msg);
  
  //sendData(msg);
  
}


void loop() {

  
  checkMotion();
  
  timer.run();
  
  

}



//void sendData(String message) {
// 
//     mydata.sensor = 0;
//     mydata.reading = 0;
//
//    int len = message.length()+1;
//    
//    if (len>50)
//    {
//      len = 50;
//    }
//    
//    message.toCharArray(mydata.message, 50);
//     
//     Serial.print("sending serial message: ");
//     Serial.println(mydata.message);
//     
//     //send the data
//     ET.sendData(); 
//  
//}

void sendData(byte sensor, int reading) {
 
     mydata.sensor = sensor;
     mydata.reading = abs(reading);
    
    mydata.readingNegative = reading<0;
    
//    for (int i=0;i<50;i++) {
//         mydata.message[i] = 0;
//    }

     //send the data
     ET.sendData(); 
  
}


void checkMotion() {
  
  
  boolean motionvalue = digitalRead(motionpin) == HIGH;
  
  if (motionvalue && !lastMotion) {

     lastMotionAt = millis();

     digitalWrite(ledpin, HIGH);
     ledOn = true;
     ledOnAt = millis();
   
     Serial.println("w 9 0");

     sendData(9,0);
      
  }
  else if (lastMotion && !motionvalue) {
    
     Serial.println("w 9 1");    
     //mySerial.println("w 9 1");    
     digitalWrite(ledpin, LOW);
     
     sendData(9,1);
  }
  
  lastMotion = motionvalue;
  
}

void checkLightLevelBig() {
  
  checkLightLevel(100);
  
}

void checkLightLevelSmall() {
  
  checkLightLevel(10);
  
}

void checkLightLevel(int minChange) {
  
  Serial.print("checkLightLevel ");

  int lightvalueNew = int(analogRead(lightpin)/1024.0*100)*10;

  Serial.println(lightvalueNew);
    
//    memcpy(&lightvals[0], &lightvals[1], (9)*sizeof(*lightvals));
//    lightvals[9] = lightvalueNew;
//
//    
//    for (int x=0;x<10;x++) {
//      
//      Serial.print(lightvals[x]);
//      Serial.print('\t');
//    }
//    
//    Serial.println();
  
  
  
  
  int lightChange = abs(lightvalueNew-lightvalue);
  //only if changed by >= 10
  if (lightChange>=minChange) {

     delay(50);
     lightvalueNew = int(analogRead(lightpin)/1024.0*100)*10;
  
      lightChange = abs(lightvalueNew-lightvalue);
      //only if changed by >= 10
      if (lightChange>=minChange) {
      
       //and at most every 10 seconds or > 100 change
       //if (lightChange>=100) 
//       {
//         lightvalueAt = millis();
         lightvalue = lightvalueNew;
         
         Serial.print("w 10 ");
         Serial.println(lightvalueNew);
         
         sendData(10,lightvalueNew);
       //}
    }
  }

    
  
}

void initTemperatures() {
  
  Serial.println("initTemperatures");

  pinMode(tempground, OUTPUT);
  digitalWrite(tempground, LOW);
    
  delay(5000);
  
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
    //temperature[i]=-100;
    
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
	{
		Serial.print("Found device ");
		Serial.print(i, DEC);
		//Serial.print(" with address: ");
//		printAddress(tempDeviceAddress);
		Serial.println();
		
		//Serial.print("Setting resolution to ");
		//Serial.println(TEMPERATURE_PRECISION, DEC);
		
		// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
		//sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
		
		// Serial.print("Resolution actually set to: ");
		//Serial.print(sensors.getResolution(tempDeviceAddress), DEC); 
		//Serial.println();


             
	}else{
  
  
		Serial.print("Found ghost device at ");
		Serial.print(i, DEC);
		Serial.print(" but could not detect address. Check power and cabling");

             
	}
  }
  
}


void checkTemperatures() {
  
  Serial.println("checkTemperatures");
  
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println("DONE");
  
  int numberOfDevices = sensors.getDeviceCount();  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++)
  {
         
    //String msg = "checking temp for sensor ";
    //msg += i;
    //sendData(msg); 
     
    float fahrenheit = sensors.getTempFByIndex(i);
    
      
    // Search the wire for address
    if(fahrenheit != DEVICE_DISCONNECTED_F)
    {
      
      //if (abs(temperature[i]-fahrenheit)>=1) {
    
          temperature[i] = fahrenheit;
       
         Serial.print("w ");
         Serial.print(i+tempDeviceStart);
         Serial.print(" ");
         Serial.println(int(fahrenheit));
         
          sendData(i+tempDeviceStart, int(fahrenheit));
             
//      }
//      else {
//       
//    
//     sendData("temp unchanged"); 
//       //Serial.println("temp unchanged");     
//        
//      }
    }
    else {
      
      Serial.println("Temp Sensor disconnected");
     //sendData("Temp Sensor disconnected"); 
      
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
