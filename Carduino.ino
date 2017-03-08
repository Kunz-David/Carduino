#include <IRremote.h>
#include <SD.h>
#include <SPI.h>
#include <PrintEx.h>

//libraries used by Motor Shield
#include <Wire.h>
#include <Adafruit_MotorShield.h>
//#include "utility/Adafruit_MS_PWMServoDriver.h"

//create a file to write to for testing
File myFile;
//SD card
const int csPin = 53;

//printf
StreamEx mySerial = Serial;

//pins
const int trigPinL = 33;
const int echoPinL = 31;
const int trigPinR = 37;
const int echoPinR = 35;

// ultrasonic variables
long durationLeft;
int distanceLeft;
long durationRight;
int distanceRight;

//motorshield object
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motorL = AFMS.getMotor(1);
Adafruit_DCMotor *motorR = AFMS.getMotor(2);

//loop logic
boolean running = true;

//motor variables
float speedRightCoefficient = 0.0;
float speedLeftCoefficient = 0.0;
uint8_t directionRight = RELEASE;
uint8_t directionLeft = RELEASE;

//IR reciever
const int receiverPin = 24;
IRrecv irrecv(receiverPin);
decode_results results;

//gear speed
int motorSpeed = 150;

boolean disableDistanceSlow = false;

void setup() {
  Serial.begin(9600);

  //ultrasonic
  pinMode(trigPinL, OUTPUT);
  pinMode(echoPinL, INPUT);
  pinMode(trigPinR, OUTPUT);
  pinMode(echoPinR, INPUT);

  //SD card
  pinMode(csPin, OUTPUT);

  //motors
  AFMS.begin();

  //IR reciever
  irrecv.enableIRIn();
}

void loop() {

  if (irrecv.decode(&results)) {
    switch (results.value){
      case 0xFF18E7: // Go straight (button ▲)
        setMovementVars(1.0, 1.0, FORWARD, FORWARD);
        log("CMD: Go straight");
        break;
      case 0xFF5AA5: // Turn right (button ►)
        setMovementVars(2.0/3.0, 0.0, FORWARD, FORWARD);
        log("CMD: Turn right");
        break;
      case 0xFF10EF: // Turn left (button ◄)
        setMovementVars(0.0, 2.0/3.0, FORWARD, FORWARD);
        log("CMD: Turn left");
        break;
      case 0xFF4AB5: // Go backwards (button ▼)
        setMovementVars(2.0/3.0, 2.0/3.0, BACKWARD, BACKWARD);
        log("CMD: Go backward");
        break;
      case 0xFF38C7: // release (button OK)
        setMovementVars(0.0, 0.0, RELEASE, RELEASE);
        break;
      //CHANGE SPEED:
      case 0xFFA25D: // speed gear 1 (button 1)
        motorSpeed = 43;
        log("CMD: Gear 1");
        break;
      case 0xFF629D: // speed gear 2 (button 2)
        motorSpeed = 85;
        log("CMD: Gear 2");
        break;
      case 0xFFE21D: // speed gear 3 (button 3)
        motorSpeed = 128;
        log("CMD: Gear 3");
        break;
      case 0xFF22DD: // speed gear 4 (button 4)
        motorSpeed = 170;
        log("CMD: Gear 4");
        break;
      case 0xFF02FD: // speed gear 5 (button 5)
        motorSpeed = 213;
        log("CMD: Gear 5");
        break;
      case 0xFFC23D: // speed gear 6 (button 6)
        motorSpeed = 255;
        log("CMD: Gear 6");
        break;
      case 0xFF6897: // slow down (button *)
        motorSpeed = motorSpeed - 20;
        if(motorSpeed < 0) motorSpeed = 0;
        break;
      case 0xFFB04F: // speed up (button #)
        motorSpeed = motorSpeed + 20;
        if(motorSpeed > 255) motorSpeed = 255;
        break;
      case 0xFFE01F: // print everything (button 7)
        break;
    }

    runMotors();

    irrecv.resume();
  }

  updateDistance();

return;
  if(distanceLeft<25){
    setMovementVars(2.0/3.0, 2.0/3.0, directionLeft, directionRight);
    runMotors();
    disableDistanceSlow = true;
  }
}
/**
 * [setMovementVars description]
 * @param speedL     [description]
 * @param speedR     [description]
 * @param directionL [description]
 * @param directionR [description]
 */
void setMovementVars(float speedL, float speedR, uint8_t directionL, uint8_t directionR){
  speedLeftCoefficient = speedL;
  speedRightCoefficient = speedR;
  directionLeft = directionL;
  directionRight = directionR;
}

void runMotors(){
  motorR->setSpeed(speedRightCoefficient*motorSpeed);
  motorL->setSpeed(speedLeftCoefficient*motorSpeed);
  motorR->run(directionRight);
  motorL->run(directionLeft);
}

void log(String command){
  Serial.println(command);
  mySerial.printf("Data: \n\tSpeedLeftCoefficient: %.2f,\n\tspeedRightCoefficient: %.2f,\n\tdirectionLeft: %d,\n\tdirectionRight: %d,\n\tdistanceLeft: %d,\n\tdistanceRight: %d\n", speedLeftCoefficient, speedRightCoefficient, directionLeft, directionRight, distanceLeft, distanceRight);
  myFile = SD.open("log.txt", FILE_WRITE);
  myFile.println(command);
  myFile.close();
}

void updateDistance(){
  // Clears the trigPin
  digitalWrite(trigPinL, LOW);
  digitalWrite(trigPinR, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPinL, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinL, LOW);
  durationLeft = pulseIn(echoPinL, HIGH);

  digitalWrite(trigPinR, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinR, LOW);
  durationRight = pulseIn(echoPinR, HIGH);

  // Calculating the distance
  distanceLeft = durationLeft*0.034/2;
  distanceRight = durationRight*0.034/2;
  // Prints the distance on the Serial Monitor
  /*
  Serial.print("L: ");
  Serial.print(distanceLeft);
  Serial.print("  R: ");
  Serial.println(distanceRight);
  */
}
