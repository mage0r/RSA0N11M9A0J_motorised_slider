/*

  Control menu for I2C motorised slider control
  John Spencer - 2014
  https://github.com/mage0r/RSA0N11M9A0J_motorised_slider
  
  Adapted from:
  http://www.forkrobotics.com/2012/05/relay-control-over-i2c/
  
  
  
*/

#include <Wire.h>
int selection=0;
int ATtinyAddress=0x26;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup(){
    Wire.begin();
    Serial.begin(9600);
    inputString.reserve(200);
    PrintMenu();
}
void loop(){
  
  char inSerial; // temporty serial character
  
  if(Serial.available()){
     inSerial = (char)Serial.read();
     
       inputString += inSerial;
     
     if (inSerial == ';')
       stringComplete = true;
   }
   
   if (stringComplete) {
     if ( inputString.charAt(0) == 'A' ) {
       ATtinyAddress = inputString.substring(1,inputString.length()-1).toInt();
       Serial.print("Writing to device: ");
       Serial.println(ATtinyAddress);
     }
     else
     {
       SendI2C(ATtinyAddress,inputString);
     }
     
     inputString = "";
     stringComplete = false;
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
