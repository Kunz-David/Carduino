#include <IRremote.h>
#include <PrintEx.h>
#include <NewPing.h>
#include <toneAC.h> //use pins 11 & 12 pwm

//libraries used by Motor Shield
#include <Wire.h>
#include <Adafruit_MotorShield.h>
//#include "utility/Adafruit_MS_PWMServoDriver.h"

//logging
StreamEx mySerial = Serial;
void log(String command, boolean logOn = true);
boolean loggingOn = true;

// ultrasonic variables
#define NUMBER_OF_SONARS 3      // Number of sensors.
#define MAX_MEASURE_DISTANCE 200 // Maximum distance (in cm) to ping.
NewPing sonar[NUMBER_OF_SONARS] = {   // Sensor object array.
  NewPing(35, 33, MAX_MEASURE_DISTANCE), //left sonar
  NewPing(41, 39, MAX_MEASURE_DISTANCE), // middle sonar
  NewPing(47, 45, MAX_MEASURE_DISTANCE)  //right sonar
};
int distance[NUMBER_OF_SONARS];

//motorshield object
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motorL = AFMS.getMotor(1);
Adafruit_DCMotor *motorR = AFMS.getMotor(2);

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

void setMovementVars(float spdL, float spdR, uint8_t direcL, uint8_t direcR){
  speedLeftCoefficient = spdL;
  speedRightCoefficient = spdR;
  directionLeft = direcL;
  directionRight = direcR;
}

void log(String command, boolean logOn = true){
  if (logOn == true){
    float secs = millis()/1000.0;
    Serial.println(command);
    mySerial.printf("Data:\t(%.1f)\n\tSpeedLeftCoefficient: %.2f\n\tspeedRightCoefficient: %.2f\n\tdistance Left: %d\n\tdistance Middle: %d\n\tdistance Right: %d\n",
                                secs, speedLeftCoefficient, speedRightCoefficient, distance[0], distance[1], distance[2]);
  }
}

void updateDistance(){
  for (uint8_t i = 0; i < NUMBER_OF_SONARS; i++){
    distance[i] = sonar[i].ping_cm();
  }
}

void runMotors(float spdPer){
  motorR->setSpeed(speedRightCoefficient*motorSpeed*spdPer);
  motorL->setSpeed(speedLeftCoefficient*motorSpeed*spdPer);
  motorR->run(directionRight);
  motorL->run(directionLeft);
}

class ChangeSpeed
{
	// Class Member Variables
	// These are initialized at startup
	float speedPercentage;
	int changeSpeedDistance;
	int resetTime;
  boolean disableChangeSpeed;
	unsigned long previousMillis;  	//stores when them speed was last modified

  //tone variables
  //toneAC( frequency [, volume [, length [, background ]]] )
  unsigned int toneFrequency;
  int toneVolume; // 0 --> turned off, 1-10 --> range
  unsigned int toneLength; //in millis
  boolean toneBackground; //Play note in background or pause till finished? (default: false, values: true/false)

  // Constructor - creates a ChangeSpeed object
  // and initializes the member variables and state
  public:
  ChangeSpeed(int chSpdDis, int resTime, int toneFreq, float spdPer) //add tone variables
  {
  changeSpeedDistance = chSpdDis;
  resetTime = resTime*1000;
  speedPercentage = spdPer;

  //toneVariables:
  toneFrequency = toneFreq;
  toneVolume = 5;
  toneLength = 500;
  toneBackground = false; //already default - not needed here

	previousMillis = 0;
  disableChangeSpeed = false;
  }

  void CheckForObstacle(){ //checks for an obstacle for the actual object if its within changeSpeedDistance it makes a sound and changes the speed to speedPercentage
    unsigned long currentMillis = millis();
    if((0 < distance[0] && distance[0] <= changeSpeedDistance) || (0 < distance[1] && distance[1] <= changeSpeedDistance) || (0 < distance[2] && distance[2] <= changeSpeedDistance)){
      //add tone here:
      toneAC(toneFrequency, toneVolume, toneLength, toneBackground);
      if(disableChangeSpeed == false){
        runMotors(speedPercentage);
        disableChangeSpeed = true;
        previousMillis = currentMillis;
        log("EVENT: Change speed");
      }
    } else if(currentMillis - previousMillis >= resetTime){
      disableChangeSpeed = false;
    }
  }
};

ChangeSpeed slow(25, 3000, 10, 0.5);
ChangeSpeed stop(10, 2000, 10, 0.0);

void setup() {
  Serial.begin(9600);

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
        log("CMD: Go straight", loggingOn);
        break;
      case 0xFF5AA5: // Turn right (button ►)
        setMovementVars(1.0, 1.0/3.0, FORWARD, FORWARD);
        log("CMD: Turn right", loggingOn);
        break;
      case 0xFF10EF: // Turn left (button ◄)
        setMovementVars(1.0/3.0, 1.0, FORWARD, FORWARD);
        log("CMD: Turn left", loggingOn);
        break;
      case 0xFF4AB5: // Go backwards (button ▼)
        setMovementVars(2.0/3.0, 2.0/3.0, BACKWARD, BACKWARD);
        log("CMD: Go backward", loggingOn);
        break;
      case 0xFF38C7: // release (button OK)
        setMovementVars(0.0, 0.0, RELEASE, RELEASE);
        break;
      //CHANGE SPEED:
      case 0xFFA25D: // speed gear 1 (button 1)
        motorSpeed = 43;
        log("CMD: Gear 1", loggingOn);
        break;
      case 0xFF629D: // speed gear 2 (button 2)
        motorSpeed = 85;
        log("CMD: Gear 2", loggingOn);
        break;
      case 0xFFE21D: // speed gear 3 (button 3)
        motorSpeed = 128;
        log("CMD: Gear 3", loggingOn);
        break;
      case 0xFF22DD: // speed gear 4 (button 4)
        motorSpeed = 170;
        log("CMD: Gear 4", loggingOn);
        break;
      case 0xFF02FD: // speed gear 5 (button 5)
        motorSpeed = 213;
        log("CMD: Gear 5", loggingOn);
        break;
      case 0xFFC23D: // speed gear 6 (button 6)
        motorSpeed = 255;
        log("CMD: Gear 6", loggingOn);
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
        log("CMD: Check data");
        break;
    }
    runMotors(1.0);

    irrecv.resume();
  }

  updateDistance();

  slow.CheckForObstacle();
  stop.CheckForObstacle();
}
