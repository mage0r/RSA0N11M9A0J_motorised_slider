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
 
#include "OnePinCapSense.h"
 #include <Adafruit_NeoPixel.h>
 
 
boolean MoveSlider(int moveTo, int currentPosition, int motorIAPin, int motorIBPin, int deadZone, int maxSpeed, int minSpeed );
boolean UpdateDisplay(int number);

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
int baseLine = 0 ;
int touched = 0 ;
const int offset = 50 ;
OnePinCapSense opcs = OnePinCapSense();

// movement variables.
unsigned int moveTo; // value to set the slider too.
boolean moveSlider = false; // do we need to move the slider?
unsigned int currentPosition; // previous 3 rolling values.
unsigned long update; // 
unsigned int sliderDeadZone = 10;

unsigned long yay_timer;
unsigned long yay_timer_2;
boolean yay = false;
unsigned int moveTo_test;

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
  
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  
  pinMode(neopixel, OUTPUT);
  
  update = millis();

  calibrate() ;
  
  // Calibrate the cap sense.
  //cap_touch.set_CS_AutocaL_Millis(0xFFFFFFFF);
  
  // set expected position to current position.
  //moveTo = analogRead(sensorPin);
  moveTo = 500;
  
  strip.begin();
  strip.show();


}

void loop() {

  if(currentPosition - analogRead(sensorPin) > sliderDeadZone || analogRead(sensorPin) - currentPosition > sliderDeadZone)
    currentPosition = analogRead(sensorPin);
  
  // Update the display with our current position
  // Currently the display only supports between 0 and 999
  UpdateDisplay(currentPosition);
  
  // Before updating anything, check if we've been touched or not.
  if (opcs.readCapacitivePin(5) > touched) {
      // On detect, this will resume movement after 5 seconds (hard coded below)
    // update = millis();
    // As an alternative, just stop moving.
    moveSlider = false;
    strip.setPixelColor(0, strip.Color(0, 255, 0));
      strip.setPixelColor(1, strip.Color(0, 255, 0));
  }

  //moveSlider = MoveSlider(moveTo_test, analogRead(sensorPin), motorIAPin, motorIBPin, sliderDeadZone, 200, 200);

  /*
  
  if (moveTo-currentPosition < sliderDeadZone || currentPosition-moveTo < sliderDeadZone) {
      // we're where we expect to be!  happy green.
      strip.setPixelColor(0, strip.Color(0, 255, 0));
      strip.setPixelColor(1, strip.Color(0, 255, 0));
      yay = true;
      yay_timer = millis();
  } else if (moveSlider && update < millis()-5000) {
    // We'd like to move, but also check that a timer trigger hasn't happened.
    // At 5v, I find anything less than 150 won't move the sliders
    // do a fresh analogRead here
    moveSlider = MoveSlider(moveTo_test, analogRead(sensorPin), motorIAPin, motorIBPin, sliderDeadZone, 200, 200);
    
    // Set our colour to be "automation in progress" scheme
    strip.setPixelColor(0, strip.Color(0, 0, 255));
    strip.setPixelColor(1, strip.Color(0, 0, 255));
  } else if(!yay) {
    // We want you to move manually!
      strip.setPixelColor(0, strip.Color(255, 0, 0));
      strip.setPixelColor(1, strip.Color(255, 0, 0));
  }
  else if (yay_timer < millis()-5000) {
    uint16_t i, j;

    moveTo = random (10,1000);

    for(j=0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      delay(10);
    }

    yay = false;
    moveTo_test = random(10,1000);
    moveSlider = true;
  }

  */

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

int calibrate()
{
  int sample = 0 ;
  
  for(int i = 2 ; i < 11 ; i+=2)
  {
     // take 30 samples and return the highest value for the pin
     for( int j = 0 ; j < 30 ; j++ )
     {
       sample = opcs.readCapacitivePin(i) ;
       if( sample > baseLine)
       {
         baseLine = sample ;
       }
     }
  }
  touched = baseLine + offset ;
} // end of calibrate

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
