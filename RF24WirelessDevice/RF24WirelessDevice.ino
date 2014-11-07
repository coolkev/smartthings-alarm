
 /** RF24Mesh_Example.ino by TMRh20
  * 
  * This example sketch shows how to manually configure a node via RF24Mesh, and send data to the
  * master node.
  * The nodes will refresh their network address as soon as a single write fails. This allows the
  * nodes to change position in relation to each other and the master node.
  */


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
//#include <printf.h>


/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(8,10);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

/** 
 * User Configuration: nodeID - A unique identifier for each radio. Allows addressing
 * to change dynamically with physical changes to the mesh.  
 *
 * In this example, configuration takes place below, prior to uploading the sketch to the device
 * A unique value from 1-255 must be configured for each node. 
 * This will be stored in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.
 *
 **/
#define nodeID 1
    


uint32_t displayTimer=0;

int buttonPin = 3;
boolean buttonPressed = false;
int ledPin = 4;

void setup() {
  
  Serial.begin(57600);
  
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(ledPin, HIGH);
  
  //printf_begin();
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  mesh.begin();  
  
  for (int x=0;x<3;x++) {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
  }
  digitalWrite(ledPin, LOW);
  
  Serial.println(F("Connected"));
  pinMode(buttonPin, INPUT_PULLUP);
}



void loop() {
  
  mesh.update();

  boolean newButtonPressed = digitalRead(buttonPin) == LOW;
  
  if (newButtonPressed != buttonPressed) {
    
    if (newButtonPressed) {
      Serial.println("Button Pressed ");
    }
    else {
       Serial.println("Button Released ");
    }
  
    //payload_t payload = { 1,  };
//    RF24NetworkHeader header(other_node);
  //  bool ok = network.write(header,&payload,sizeof(payload));
  
    byte reading = newButtonPressed ? 1 : 0;
  
    if(!mesh.write(&reading,'1',sizeof(reading))){
         
        // If a write fails, check connectivity to the mesh network
        if( ! mesh.checkConnection() ){
          //refresh the network address
          Serial.println("Renewing Address");
          mesh.renewAddress(); 
        }else{
          Serial.println("Send fail, Test OK"); 
        }
      }else{
        Serial.print("Send OK: ");
      }
    
    buttonPressed = newButtonPressed;
    
  }
  // Send to the master node every second
//  if(millis() - displayTimer >= 1000){
//    displayTimer = millis();    
    
    
    
    
    // Send an 'M' type message containing the current millis()
//    if(!mesh.write(&displayTimer,'M',sizeof(displayTimer))){
//       
//      // If a write fails, check connectivity to the mesh network
//      if( ! mesh.checkConnection() ){
//        //refresh the network address
//        Serial.println("Renewing Address");
//        mesh.renewAddress(); 
//      }else{
//        Serial.println("Send fail, Test OK"); 
//      }
//    }else{
//      Serial.print("Send OK: "); Serial.println(displayTimer);
//    }
//  }

}







