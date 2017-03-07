#include <IRremote.h>
#include <SD.h>
#include <SPI.h>

//libraries used by Motor Shield
#include <Wire.h>
#include <Adafruit_MotorShield.h>
//#include "utility/Adafruit_MS_PWMServoDriver.h"

//create a file to write to for testing
File myFile;
//SD card
const int csPin = 53;

//pins
const int trigPinL = 33;
const int echoPinL = 31;
const int trigPinR = 37;
const int echoPinR = 35;

// ultrasonic variables
long durationL;
int distanceL;
long durationR;
int distanceR;

//motorshield object
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motorL = AFMS.getMotor(1);
Adafruit_DCMotor *motorR = AFMS.getMotor(2);

//loop logic
boolean running = true;

//motor variables
int speedR;
int speedL;
String directionR;
String directionL;

//IR reciever
const int receiverPin = 24;
IRrecv irrecv(receiverPin);
decode_results results;

//gear speed
int motorSpeed = 150;


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

  motorL->setSpeed(50);
  motorL->run(FORWARD);
  motorL->run(RELEASE);
  motorR->setSpeed(50);
  motorR->run(FORWARD);
  motorR->run(RELEASE);
}
void loop() {

  if (irrecv.decode(&results)) {
      switch (results.value){
        case 0xFF18E7: // Go straight (button ▲)
          setMovement(motorSpeed, motorSpeed, FORWARD, FORWARD);
          log("CMD: Go straight");
          Serial.println("straight");
          break;
        case 0xFF5AA5: // Turn right (button ►)
          setMovement(motorSpeed*2/3, 0, FORWARD, FORWARD);
          log("CMD: Turn right");
          break;
        case 0xFF10EF: // Turn left (button ◄)
          setMovement(0, motorSpeed*2/3, FORWARD, FORWARD);
          log("CMD: Turn left");
          break;
        case 0xFF4AB5: // Go backwards (button ▼)
          setMovement(motorSpeed*2/3, motorSpeed*2/3, BACKWARD, BACKWARD);
          log("CMD: Go backward");
          break;
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
        case 0xFF38C7: // release (button OK)
          setMovement(0, 0, RELEASE, RELEASE);
          break;
        case 0xFF6897: // slow down (button *)
          motorSpeed = motorSpeed - 20;
          break;
        case 0xFFB04F: // speed up (button #)
          motorSpeed = motorSpeed + 20;
          break;
      }
    irrecv.resume();
  }
  //updateDistance();

}

void setMovement(int speedL, int speedR, uint8_t directionL, uint8_t directionR){
  motorR->setSpeed(speedR);
  motorL->setSpeed(speedL);
  motorR->run(directionR);
  motorL->run(directionL);
}

void log(String data){
  myFile = SD.open("log.txt", FILE_WRITE);
  myFile.println(data);
  myFile.close();
  Serial.println(data);
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
  durationL = pulseIn(echoPinL, HIGH);

  digitalWrite(trigPinR, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinR, LOW);
  durationR = pulseIn(echoPinR, HIGH);

  // Calculating the distance
  distanceL = durationL*0.034/2;
  distanceR = durationR*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("L: ");
  Serial.print(distanceL);
  Serial.print("  R: ");
  Serial.println(distanceR);
}
