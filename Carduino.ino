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
Adafruit_DCMotor *motorL = AFMS.getMotor(2);
Adafruit_DCMotor *motorR = AFMS.getMotor(1);

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
    if (running){
      Serial.println(running);
      Serial.println("go");
      switch (results.value){
        case 0xFF18E7: // Go straight (▲ button)
          setMovement(150, 150, FORWARD, FORWARD);
          log("CMD: Go straight");
          Serial.println("straight");
          break;
        case 0xFF5AA5: // Turn right (► button)
          setMovement(100, 0, FORWARD, FORWARD);
          log("CMD: Turn right");
          break;
        case 0xFF10EF: // Turn left (◄ button)
          setMovement(0, 100, FORWARD, FORWARD);
          log("CMD: Turn left");
          break;
        case 0xFF4AB5: // Go backwards (▼ button)
          setMovement(100, 100, BACKWARD, BACKWARD);
          log("CMD: Go backward");
          break;
      }
    }
    if (results.value == 0xFF38C7) { // Stop (OK button)
     setMovement(0, 0, RELEASE, RELEASE);
    }
      /*if (results.value == 0xFF6897){
        running = true;
      }*/
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
/*
void setMovement(uint8_t directionL, uint8_t directionR){
  motorR->run(directionR);
  motorL->run(directionL);
}
*/
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
