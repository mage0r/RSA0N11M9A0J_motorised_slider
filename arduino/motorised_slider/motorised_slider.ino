/*
  This test code moves a motor based slider up and down using a motor driver.
  John Spencer - 2014
  https://github.com/mage0r/RSA0N11M9A0J_motorised_slider
  
  You'll need the libraries from the following locations.
  https://github.com/adafruit/Adafruit_NeoPixel
  http://playground.arduino.cc/Code/USIi2c
 */
 
 /* To Do
  * ~~~~~
  * - Store / recall I2C address in EEPROM.
  */
 
 #include <CapacitiveSensor.h>
 #include <Adafruit_NeoPixel.h>
 #include "TinyWireS.h"
 
 #include <EEPROM.h>
 
// i2c Address stuff.
#define i2c_eeprom_address 0 // Always gonna be the first bit.

byte i2c_slave_addr;
 
boolean MoveSlider(int moveTo, int currentPosition, int motorIAPin, int motorIBPin, int deadZone, int maxSpeed, int minSpeed );
boolean UpdateDisplay(int number);
void CalibrateCap(int offset);

// Pin variables.
const int capacitivePin = A1; // this is the send pin for capacitive touch
const int motorIAPin = 7;
const int motorIBPin = 8;
const int sensorPin = A0;
const int neopixel = 10;

// serial out for 7 segment
const int clockPin = 3;
const int dataPin = 2; // SER
const int latchPin = 9;

// Cap sense variables.
CapacitiveSensor cap_touch = CapacitiveSensor(5,1);

// movement variables.
unsigned int moveTo; // value to set the slider too.
boolean moveSlider = false; // do we need to move the slider?
unsigned int currentPosition; // previous 3 rolling values.
unsigned long update; // 
unsigned int sliderDeadZone = 10;

// data string input
String inputString = "";
boolean stringComplete = false;

// This is an array of the different digit combinations
const byte digits [11][2] =
{
  {0b00000110,0b00000000}, //0
  {0b00000111,0b11001000}, //1
  {0b00000101,0b00100000}, //2
  {0b00000101,0b10000000}, //3
  {0b00000100,0b11001000}, //4
  {0b00000100,0b10010000}, //5
  {0b00000100,0b00010000}, //6
  {0b00000111,0b11000000}, //7
  {0b00000100,0b00000000}, //8
  {0b00000100,0b11000000}, //9
  {0b00000011,0b11111000}, //.
};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, neopixel, NEO_GRB + NEO_KHZ800);

void setup() {
  
  // get i2c address.  If one doesn't exist, set the default.
  i2c_slave_addr = EEPROM.read(i2c_eeprom_address);
  if(!i2c_slave_addr || i2c_slave_addr == 255) { //empty address
    EEPROM.write(i2c_eeprom_address, 38); // we're defaulting to 38
    i2c_slave_addr = 38;
  }
  
  TinyWireS.begin(i2c_slave_addr);
  
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  
  pinMode(neopixel, OUTPUT);
  
  update = millis();
  
  // Calibrate the cap sense.
  //cap_touch.set_CS_AutocaL_Millis(0xFFFFFFFF);
  
  // set expected position to current position.
  moveTo = analogRead(sensorPin);
  
  inputString.reserve(50);
  
  strip.begin();
  strip.show();
}

void loop() {

  char inI2C; // i2c temp variable.
  
  if (TinyWireS.available()){           // got I2C input!
  
    inI2C = (char)TinyWireS.receive();     // get the byte from master
    // ; is the end command string.
      if(inI2C != ';'){
        inputString += inI2C;
      }
      else
        stringComplete = true;
  }

  moveTo = 800;
  moveSlider = true;

  // We've received a full command.  Match it to our existing members.
  if(stringComplete) {
    
    if( inputString.charAt(0) == 'M' ){
      // Move the motor to a set position
      moveTo = inputString.substring(1).toInt();
      moveSlider = true;
    }
    else if ( inputString.charAt(0) == 'R' ) {
      i2c_slave_addr = inputString.substring(2,inputString.length()).toInt();
      EEPROM.write(i2c_eeprom_address, i2c_slave_addr);
      TinyWireS.begin(i2c_slave_addr);
    }
    else if ( inputString.charAt(0) == 'U' ) {
      // move the slider up a bit.
      moveTo = moveTo + inputString.substring(1).toInt();
      moveSlider = true;
    }
    else if ( inputString.charAt(0) == 'D' ) {
      // move the slider down a bit.
      moveTo = moveTo - inputString.substring(1).toInt();
      moveSlider = true;
    }
    else if ( inputString.charAt(0) == 'C' ) {
      // re-run the capsense calibration.
      CalibrateCap(inputString.substring(1).toInt());
    }
    else if ( inputString.charAt(0) == 'Z' ) {
      // adjust the deadzone for movement.
      sliderDeadZone = inputString.substring(1).toInt();
    }
    else if( inputString.charAt(0) == 'E' ){
      // Flag error and get user to move to position.
      moveTo = inputString.substring(1).toInt(); 
    }
    else if( inputString.charAt(0) == 'P' ){
      // Request for Position information
      // reuse inputString for output - ugly.
      inputString = "P";
      inputString += String(analogRead(sensorPin));
      inputString += ";";
      char buffer[32];
      inputString.toCharArray(buffer,32);
      for(int i = 0; i < 32; i++) {
        if(buffer[i])
          TinyWireS.send(buffer[i]);
        else
          break;
      }
    }
    else if( inputString.charAt(0) == 'S' ){
      // Request for current settings.
      // Current Position, Dead Zone, CapSense offset and speed offset
      inputString = "P";
      inputString += String(analogRead(sensorPin));
      inputString += ";";
      inputString += "Z";
      inputString += String(sliderDeadZone);
      inputString += ";";
      char buffer[32];
      inputString.toCharArray(buffer,32);
      for(int i = 0; i < 32; i++) {
        if(buffer[i])
          TinyWireS.send(buffer[i]);
        else
          break;
      }
      
    }
    
    // Clear the stream out for the next command.
    inputString = "";
    stringComplete = false;
  }
  
  
  if(currentPosition - analogRead(sensorPin) > sliderDeadZone || analogRead(sensorPin) - currentPosition > sliderDeadZone)
    currentPosition = analogRead(sensorPin);
  
  // Update the display with our current position
  // Currently the display only supports between 0 and 999
  UpdateDisplay(currentPosition);
  
  // Before updating anything, check if we've been touched or not.
  if (cap_touch.capacitiveSensor(5) > baseLine) {
      // On detect, this will resume movement after 5 seconds (hard coded below)
    // update = millis();
    // As an alternative, just stop moving.
    moveSlider = false;
  }
  
  
  if (moveTo-currentPosition < sliderDeadZone || currentPosition-moveTo < sliderDeadZone) {
      // we're where we expect to be!  happy green.
      strip.setPixelColor(0, strip.Color(0, 20, 0));
      strip.setPixelColor(1, strip.Color(0, 20, 0));
  } else if (moveSlider && update < millis()-5000) {
    // We'd like to move, but also check that a timer trigger hasn't happened.
    // At 5v, I find anything less than 150 won't move the sliders
    // do a fresh analogRead here
    moveSlider = MoveSlider(moveTo, analogRead(sensorPin), motorIAPin, motorIBPin, sliderDeadZone, 200, 200);
    
    // Set our colour to be "automation in progress" scheme
    strip.setPixelColor(0, strip.Color(0, 0, 20));
    strip.setPixelColor(1, strip.Color(0, 0, 20));
  } else {
    // We want you to move manually!
      strip.setPixelColor(0, strip.Color(20, 0, 0));
      strip.setPixelColor(1, strip.Color(20, 0, 0));
  }

  // update our colour strip
  strip.show();
}

// update the 7 segment display
// pretty much have to run this each iteration
boolean UpdateDisplay(int displayNumber) {
  
  short ones=0, tens=0, hundreds=0;

  hundreds = int(displayNumber/100);
  
  tens = int((displayNumber%100)/10);
  
  ones = int(displayNumber-(hundreds*100)-(tens*10));
  
  // write 100's
  digitalWrite(latchPin, LOW);
  // Send the value as a binary sequence to the module
  shiftOut(dataPin, clockPin, MSBFIRST, digits[hundreds][0]);
  shiftOut(dataPin, clockPin, MSBFIRST, (digits[hundreds][1]+0b00000100)); // binary addition here sets the segment
  // Enable the latch again to set the output states
  digitalWrite(latchPin, HIGH);
  
  // write 10's
  digitalWrite(latchPin, LOW);
  // Send the value as a binary sequence to the module
  shiftOut(dataPin, clockPin, MSBFIRST, digits[tens][0]);
  shiftOut(dataPin, clockPin, MSBFIRST, (digits[tens][1]+0b00000010)); // binary addition here sets the segment
  // Enable the latch again to set the output states
  digitalWrite(latchPin, HIGH);
  
  // write 1's
  digitalWrite(latchPin, LOW);
  // Send the value as a binary sequence to the module
  shiftOut(dataPin, clockPin, MSBFIRST, digits[ones][0]);
  shiftOut(dataPin, clockPin, MSBFIRST, (digits[ones][1]+0b00000001)); // binary addition here sets the segment
  // Enable the latch again to set the output states
  digitalWrite(latchPin, HIGH);
  
  // switch off.  Keeps the LED's at a consistent brightness.
  digitalWrite(latchPin, LOW);
  // Send the value as a binary sequence to the module
  shiftOut(dataPin, clockPin, MSBFIRST, 0b00000111);
  shiftOut(dataPin, clockPin, MSBFIRST, 0b11111000); // binary addition here sets the segment
  // Enable the latch again to set the output states
  digitalWrite(latchPin, HIGH);
  
  return true; 
}

// This method moves the slider to requested position.
// It handles acceleration and dead zone to prevent bouncing.
boolean MoveSlider(int moveTo, int currentPosition, int motorIAPin, int motorIBPin, int deadZone = 10, int maxSpeed = 255, int minSpeed = 200) {
  // hacky while I work out how to do this.
  int motorSpeed = maxSpeed;

  // lets cheat a bit to do the deadzone stuff.
  moveTo = map(moveTo, 1, 1023, 0, 1024/deadZone);
  currentPosition = map(currentPosition, 1, 1023, 0, 1024/deadZone);
  
    if ( moveTo < currentPosition ){
      if ( ( currentPosition - moveTo ) < 2*deadZone )
        motorSpeed = minSpeed;
      analogWrite(motorIAPin, LOW);
      analogWrite(motorIBPin, motorSpeed);
    }
    else if ( moveTo > currentPosition ){
      if ( (moveTo - currentPosition ) < 2*deadZone )
        motorSpeed = minSpeed;
      analogWrite(motorIAPin, motorSpeed);
      analogWrite(motorIBPin, LOW);
    }
    else {
        digitalWrite(motorIAPin, LOW);
        digitalWrite(motorIBPin, LOW);
        //Serial.println("slider disengadged.");
        return false;
    }
    
    return true; 
}
