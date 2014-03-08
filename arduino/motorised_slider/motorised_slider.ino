/*
  This test code moves a motor based slider up and down using a motor driver.
  John Spencer - 2014
  https://github.com/mage0r/RSA0N11M9A0J_motorised_slider
 */
 
 
 #include "OnePinCapSense.h"
 #include <Adafruit_NeoPixel.h>
 
boolean MoveSlider(int moveTo, int currentPosition, int motorIAPin, int motorIBPin, int deadZone, int maxSpeed, int minSpeed );
boolean UpdateDisplay(int number);
int Calibrate();

// Pin variables.
const int capacitivePin = A1; // this is the send pin for capacitive touch
const int motorIAPin = 7;
const int motorIBPin = 8;
const int sensorPin = A0;
const int neopixel = 10;

// serial out.
const int clockPin = 3;
const int dataPin = 2; // SER
const int latchPin = 9;

int baseLine = 0 ;
const int offset = 50 ;
int touched = 0 ;
OnePinCapSense opcs = OnePinCapSense();

int setTo; // value to set the slider too.
boolean moveSlider = false;
boolean forward = true;

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

int number=0;
unsigned long update;

void setup() {
  
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  
  pinMode(neopixel, OUTPUT);
  
  update = millis();
  
  Calibrate();
  
  strip.begin();
  strip.show();
}

void loop() {

  // This is just a simple way to send this program commands for testing.
  
  touched = opcs.readCapacitivePin(capacitivePin);
  number = map(analogRead(sensorPin),0,1024,0,100);
  UpdateDisplay(number);
  
  // simple movement
  if (forward) {
    setTo = 800;
  }
  else
  {
    setTo = 100;
  }
  
  
  digitalWrite(neopixel, LOW);
  if (touched > baseLine) {
    setTo = analogRead(sensorPin);
  }

  
  // if we're meant to be moving, run the movement subroutine.
  if (moveSlider) {
    // if the subroutine has indicated that it has finished moving.
    // At 5v, I find anything less than 150 won't move the sliders
    moveSlider = MoveSlider(setTo, analogRead(sensorPin), motorIAPin, motorIBPin, 10, 200, 200);
  }else if (update < millis()-5000){
    
    if (forward)
      forward = false;
    else
      forward = true;
      
      moveSlider = true;
      
      update = millis();
  }

  // colour strip stuff
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.setPixelColor(1, strip.Color(255, 0, 0));

}

// update the 7 segment display
// pretty much have to run this each iteration
boolean UpdateDisplay(int number) {
  
  short ones=0, tens=0, hundreds=0;

  hundreds = int(number/100);
  
  tens = int((number%100)/10);
  
  ones = int(number-(hundreds*100)-(tens*10));
  
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

// The calibrate function will find the highest quiescent sensor
// value for all of the sensors and assign that to baseline.

// It will offset to calculated baseLine and assign that to the
// touched threshold.

int Calibrate()
{
  int sample = 0 ;
  
  // take 30 samples and return the highest value for the pin
     for( int j = 0 ; j < 30 ; j++ )
     {
       sample = opcs.readCapacitivePin(capacitivePin);
       if( sample > baseLine)
       {
         baseLine = sample ;
       }
     }
     
  baseLine = baseLine + offset ;
} // end of calibrate
