/*
  This test code moves a motor based slider up and down using a motor driver.
  John Spencer - 2014
  https://github.com/mage0r/RSA0N11M9A0J_motorised_slider
 */
 
 #include <CapacitiveSensor.h>
 
boolean MoveSlider(int moveTo, int currentPosition, int motorIAPin, int motorIBPin, int deadZone, int maxSpeed, int minSpeed );

// Pin variables.
const int sendPin = 4; // this is the send pin for capacitive touch
const int receivePin = 2; // this is the recieve pin for capacitive touch.
const int motorIAPin = 3;
const int motorIBPin = 5;
const int sensorPin = A0;


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

int setTo; // value to set the slider too.
boolean moveSlider = false;

CapacitiveSensor   cs_4_2 = CapacitiveSensor(sendPin,receivePin);


void setup() {
  
  //cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  
  // initialize serial:
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {

  // This is just a simple way to send this program commands for testing.  
  char inSerial;

  if(Serial.available()){
    inSerial = (char)Serial.read();
    if (inSerial != ';'){
      inputString += inSerial;
    }
    else {
      stringComplete = true;
    }
  }

  // The serial string is complete.  run a subroutine based on that.
  if (stringComplete) {
     
    // String starting with "A" is to move Motor A;
    if ( inputString.charAt(0) == 'A' ) {
      
      setTo = inputString.substring(1).toInt();
      
      if (setTo > 0)
        moveSlider = true;
      else
        Serial.println(analogRead(sensorPin));
       
    } 
    else if ( inputString.charAt(0) == 'P' ) {
      // Output position as a Percentage.
      Serial.print("MotorA at ");
      Serial.print(map(analogRead(sensorPin),0,1024,0,100));
      Serial.println("%");
       
    }
    else if ( inputString.charAt(0) == 'C' ) {
      Serial.println(cs_4_2.capacitiveSensor(30));
    }
    else if ( inputString.charAt(0) == '?' ) {
      Serial.println("---Usage---");
      Serial.println("`?;` - This text");
      Serial.println("`A;` - Display current raw position");
      Serial.println("`A<1-1024>;` - Move the slider to position. e.g. `A800;`");
      Serial.println("`P;` - Position as a Percentage");
    }
    else {
      // just stop moving if we get junk
      moveSlider = false;
    }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
    
  }
    
  
    // check that the pots are clear to move.
  if (cs_4_2.capacitiveSensor(30) > 1000) {
 
    setTo = analogRead(sensorPin);
  }


  
  // if we're meant to be moving, run the movement subroutine.
  if (moveSlider) {
    // if the subroutine has indicated that it has finished moving.
    // At 5v, I find anything less than 150 won't move the sliders
    moveSlider = MoveSlider(setTo, analogRead(sensorPin), motorIAPin, motorIBPin, 10, 255, 200);
  }

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
        Serial.println("slider disengadged.");
        return false;
    }
    
    return true;
  
}
