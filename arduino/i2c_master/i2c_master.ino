/*

  Control menu for I2C motorised slider control
  John Spencer - 2014
  https://github.com/mage0r/RSA0N11M9A0J_motorised_slider
  
  Adapted from:
  http://www.forkrobotics.com/2012/05/relay-control-over-i2c/
  
  
  
*/

#include <Wire.h>
byte ATtinyAddress=0x26;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String receiveString = "";
boolean receiveComplete = false;

void setup(){
    Wire.begin();
    Serial.begin(9600);
    inputString.reserve(200);
    findDevices();
    PrintMenu();
    
}
void loop(){
  
  char inSerial; // temporary serial character
  char inWire;
  
  if(Serial.available()){
     inSerial = (char)Serial.read();
     
       inputString += inSerial;
     
     if (inSerial == ';')
       stringComplete = true;
   }
   
   if (stringComplete) {
     if ( inputString.charAt(0) == 'A' ) {
       ATtinyAddress = inputString.substring(1,inputString.length()-1).toInt();
       Serial.print("Writing to device: 0x");
       Serial.println(ATtinyAddress,HEX);
     }
     else if ( inputString.charAt(0) == 'F' ) {
       findDevices();
     }
     else
     {
       SendI2C(ATtinyAddress,inputString);
     }
     
     inputString = "";
     stringComplete = false;
   }
   
   if (Wire.available()) {
     inWire = (char)Wire.read();
     
       receiveString += inWire;
     
     if (inWire == ';')
       receiveComplete = true;
   }
   
   if (receiveComplete) {
     Serial.println(receiveString);
   }
  
}

void PrintMenu(){
  Serial.println("--- Relay Menu ---");
  Serial.println("e.g. M800; // move to 800");
  Serial.println("e.g. E800; // get user to move to 800");
  Serial.println("e.g. U10; // move up 10");
  Serial.println("e.g. D10; // move down 10 ");
  Serial.println("e.g. C20; // recalibrate capsense with offset 20");
  Serial.println("e.g. Z10; // deadzone is now 10");
  Serial.println("e.g. A38; // i2c address 38 (0x26)");
}

void SendI2C(byte device,String data) {
  
  char buffer[32];
  Wire.beginTransmission(device);
  data.toCharArray(buffer,32);
  Wire.write(buffer);
  Wire.endTransmission();
}

void findDevices()
{
  byte error, address;

  Serial.println("Scanning for i2c devices...");

  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
    }
  }
  Serial.println("Done Scanning.");
}
