/**
 * @file Carduino.ino
 * @Author David Kunz
 * @date March, 2017
 * @brief Code running on the Arduino Mega unit.
 * @mainpage Mainpage check
 */

//include used libraries:
//IR reciever library (https://github.com/z3t0/Arduino-IRremote)
#include <IRremote.h>
//StreamEx library (used because of the lack of pritf in Arduino) (https://github.com/Chris--A/PrintEx)
#include <PrintEx.h>
//Ultrasonic library (https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home)
#include <NewPing.h>
//Tone library; use pins 11 & 12 pwm (https://bitbucket.org/teckel12/arduino-toneac/wiki/Home)
#include <toneAC.h>
//Two Wire Interface (TWI/I2C)(used by the Adafruit Motor Shield) (http://playground.arduino.cc/Main/WireLibraryDetailedReference)
#include <Wire.h>
//AdafruitMotorShield library (https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/library-reference)
#include <Adafruit_MotorShield.h>

//logging
StreamEx mySerial = Serial;
void log(String command, boolean logOn = true);
boolean loggingOn = true;

// ultrasonic variables
#define NUMBER_OF_SONARS 3
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
#define RECEIVER_PIN 24
IRrecv irrecv(RECEIVER_PIN);
decode_results results;

//gear speed
int motorSpeed = 150;
/**
 * setMovementVars description
 * @param spdL   Speed left coefficient; percentage of power given to left motor (values 0.0-1.0)
 * @param spdR   Speed right coefficient; percentage of power given to left motor (values 0.0-1.0)
 * @param direcL Left motor direction; which direrection should the left motor run (values: FORWARD (=1), BACKWARD (=2), RELEASE (=4))(note: is relative to the wiring)
 * @param direcR Right motor direction; which direrection should the right motor run (values: FORWARD (=1), BACKWARD (=2), RELEASE (=4))(note: is relative to the wiring)
 */
void setMovementVars(float spdL, float spdR, uint8_t direcL, uint8_t direcR){
  speedLeftCoefficient = spdL;
  speedRightCoefficient = spdR;
  directionLeft = direcL;
  directionRight = direcR;
}

/**
 * Prints some current data variables on the serial monitor
 * @param command Title to the data printed
 * @param logOn   Turns logging on (true) and off (false)(note: true by default)
 */
void log(String command, boolean logOn = true){
  if (logOn == true){
    float secs = millis()/1000.0;
    Serial.println(command);
    mySerial.printf("Data:\t(%.1f)\n\tSpeedLeftCoefficient: %.2f\n\tspeedRightCoefficient: %.2f\n\tdistance Left: %d\n\tdistance Middle: %d\n\tdistance Right: %d\n",
                                secs, speedLeftCoefficient, speedRightCoefficient, distance[0], distance[1], distance[2]);
  }
}

/**
 * Saves current distances found by each sonar to "distance" array
 * note: takes 3x15ms to execute (takes 11,8ms to get ultrasonic output)
 */
void updateDistance(){
  for (uint8_t i = 0; i < NUMBER_OF_SONARS; i++){
    distance[i] = sonar[i].ping_cm();
    delay(15); //it takes 11,8ms for signal that bounces off an obstacle 200cm away to return
  }
}

/**
 * Runs the motors at the given total speed percentage.
 * @param spdPer Speed percentage; percentage of total power to both motors.
 */
void runMotors(float speedPercentage){
  motorR->setSpeed(speedRightCoefficient*motorSpeed*speedPercentage);
  motorL->setSpeed(speedLeftCoefficient*motorSpeed*speedPercentage);
  motorR->run(directionRight);
  motorL->run(directionLeft);
}

class ChangeSpeed
{
	float speedPercentage; //percentage of speed the motors should run when in slow range
	int changeSpeedDistance; //distance needed for the slow to take effect
  int endChangeSpeedDistance; //end distance of the slow effect
	int resetTime; //time that has to pass for the motors to be slow again by the same slow
  boolean disableChangeSpeed; //stores if the motors are ready to run
	unsigned long previousMillis;  	//stores when them speed was last modified

  // Constructor - creates a ChangeSpeedAt
  // and initializes the member variables and state

  //tone variables
  //tone object takes vars: toneAC( frequency [, volume [, length [, background ]]] )
  int toneFrequency;
  int toneVolume;
  int toneLength;

  public:
    /**
     * Gets
     * @param changeSpeeddDis   [description]
     * @param endChangeSpeedDis [description]
     * @param toneFreq          [description]
     * @param speedPer          [description]
     */
    ChangeSpeed(int changeSpeeddDis, int endChangeSpeedDis, int toneFreq, float speedPer) //add tone variables
      : speedPercentage(speedPer),
        changeSpeedDistance(changeSpeeddDis),
        endChangeSpeedDistance(endChangeSpeedDis),
        resetTime(5000),
        disableChangeSpeed(false),
        previousMillis(0),
        toneFrequency(toneFreq),
        toneVolume(8),
        toneLength(2)
    {
    }
/**
 * Checks if there is an obstacle in the given range (endChangeSpeedDistance-changeSpeedDistance)
 *    If there is: slows motors down to speedPercentage, plays tones (at: toneFrequency, toneFrequency and toneLength) and set a timer which waits 5s && for obstacles to be out of slow range
 *    If there isn`t: if the last speed change occured more than 5s ago it resets the timer and a new slow can occur again
 */
  void CheckForObstacle(){ //checks for an obstacle for the actual object if its within changeSpeedDistance it makes a sound and changes the speed to speedPercentage
    unsigned long currentMillis = millis();
    //opticky zmensit if, rozepsat, odentrovat, komentar
    if((0 < distance[0] && distance[0] <= changeSpeedDistance && endChangeSpeedDistance < distance[0]) || (0 < distance[1] && distance[1] <= changeSpeedDistance && endChangeSpeedDistance < distance[1]) || (0 < distance[2] && distance[2] <= changeSpeedDistance && endChangeSpeedDistance < distance[2])){
      //add tone here
      toneAC(toneFrequency, toneFrequency, toneLength);
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

//make ChangeSpeed objects
ChangeSpeed slow(35, 20, 3000, 0.5);
ChangeSpeed stop(20, 0, 3500, 0.0);

void setup() {
  Serial.begin(9600);

  //motors
  AFMS.begin();

  //IR reciever
  irrecv.enableIRIn();
}

void loop() {

  //dives into for loop if the ir reciever caught a new signal
  if (irrecv.decode(&results)) {
    switch (results.value){ //checks if the signal corresponds with any action
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
    runMotors(1.0); //runs motors at 100% (without slow)

    irrecv.resume(); //ir reciever waits for new signal input
  }

  updateDistance(); //update distance array

  //slows motor if within slow range
  slow.CheckForObstacle();
  stop.CheckForObstacle();
}
