/**
 * @file Carduino.ino
 * @Author David Kunz
 * @date March, 2017
 * @brief Code running on the Arduino Mega unit.
 */
/**
 *  @defgroup pinLayout Pin Layout
 *  @brief Layout of all pins used by name in the @ref Carduino.ino.
 */

//Include used libraries:
//IR reciever library (https://github.com/z3t0/Arduino-IRremote).
#include <IRremote.h>
//StreamEx library (used because of the lack of pritf in Arduino) (https://github.com/Chris--A/PrintEx).
#include <PrintEx.h>
//Ultrasonic library (https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home).
#include <NewPing.h>
//Tone library; use pins 11 & 12 pwm (https://bitbucket.org/teckel12/arduino-toneac/wiki/Home).
#include <toneAC.h>
//Two Wire Interface (TWI/I2C)(used by the Adafruit Motor Shield) (http://playground.arduino.cc/Main/WireLibraryDetailedReference).
#include <Wire.h>
//AdafruitMotorShield library (https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/library-reference).
#include <Adafruit_MotorShield.h>

//Logging:
/*!
 * @breif Stream extension to create a Serial that can use printf. (Arduino does not have internal support for printf.)
 */
StreamEx mySerial = Serial;
/*!
 * @breif Turn logging ON/OFF
 * @details If TRUE logging is turned ON, if FALSE logging is OFF. This doesn`t effect the dedicated log button on the remote control.
 */
boolean loggingOn = true;

//Ultrasonic/Sonar variables:
/**
 * @def NUMBER_OF_SONARS
 * Number of ultrasonic sensors connected to the Arduino board.
 */
#define NUMBER_OF_SONARS 3
/**
 * @def MAX_MEASURE_DISTANCE
 * Maximum distance to which the ultrasonic sensors will measure (i.e. how long the sensors will wait for a returning signal).
 */
#define MAX_MEASURE_DISTANCE 200 // Maximum distance (in cm) to ping.

/**
 * @defgroup sonarPins Sonar Pins
 * @ingroup pinLayout
 * @brief Ultrasonic sensor logic pin layout.
 */ 
//@{
/**Set trigger pin number for left ultrasonic sensor.*/
#define TRIG_PIN_LEFT_SONAR 35
/**Set echo pin number for left ultrasonic sensor.*/
#define ECHO_PIN_LEFT_SONAR 33
/**Set trigger pin number for middle ultrasonic sensor.*/
#define TRIG_PIN_MIDDLE_SONAR 41
/**Set echo pin number for middle ultrasonic sensor.*/
#define ECHO_PIN_MIDDLE_SONAR 39
/**Set tigger pin number for right ultrasonic sensor.*/
#define TRIG_PIN_RIGHT_SONAR 47
/**Set echo pin number for right ultrasonic sensor.*/
#define ECHO_PIN_RINGT_SONAR 45
//@}
/*!
 * @breif Initializes a NewPing sensor object array with 3 ultrasonic sensors.
 * @details Constructor: NewPing sonar(trigger_pin, echo_pin [, max_cm_distance] = 500)
 */
NewPing sonar[NUMBER_OF_SONARS] = {
  NewPing(TRIG_PIN_LEFT_SONAR, ECHO_PIN_LEFT_SONAR, MAX_MEASURE_DISTANCE), //left sonar
  NewPing(TRIG_PIN_MIDDLE_SONAR, ECHO_PIN_MIDDLE_SONAR, MAX_MEASURE_DISTANCE), // middle sonar
  NewPing(TRIG_PIN_RIGHT_SONAR, ECHO_PIN_RINGT_SONAR, MAX_MEASURE_DISTANCE)  //right sonar
};
/*!
 * @breif axaaxaaxa
 */
int distance[NUMBER_OF_SONARS];

//Motorshield object:
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motorL = AFMS.getMotor(1);
Adafruit_DCMotor *motorR = AFMS.getMotor(2);

//Motor variables:
float speedRightCoefficient = 0.0;
float speedLeftCoefficient = 0.0;
uint8_t directionRight = RELEASE;
uint8_t directionLeft = RELEASE;

//IR reciever:
/*!
 * @def RECEIVER_PIN
 * @breif IR Reciever logic pin.
 * @ingroup pinLayout
 */
#define RECEIVER_PIN 24
IRrecv irrecv(RECEIVER_PIN);
decode_results results;

//Initial gear speed:
int motorSpeed = 150;
/**
 * Set speed coefficients and direction for both motors.
 * @param spdL   Speed left coefficient; percentage of power given to left motor (values 0.0-1.0).
 * @param spdR   Speed right coefficient; percentage of power given to left motor (values 0.0-1.0).
 * @param direcL Left motor direction; which direrection should the left motor run (values: FORWARD (=1), BACKWARD (=2), RELEASE (=4))(note: is relative to the wiring).
 * @param direcR Right motor direction; which direrection should the right motor run (values: FORWARD (=1), BACKWARD (=2), RELEASE (=4))(note: is relative to the wiring).
 */
void setMovementVars(float spdL, float spdR, uint8_t direcL, uint8_t direcR){
  speedLeftCoefficient = spdL;
  speedRightCoefficient = spdR;
  directionLeft = direcL;
  directionRight = direcR;
}

/**
 * Prints some current data variables on the serial monitor.
 * @param command Title to the data printed.
 * @param logOn   Turns logging on (true) and off (false)(note: true by default).
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
 * Saves current distances found by each sonar to the distance array.
 * Note: takes 3x15ms to execute (takes 11,8ms to get ultrasonic output).
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

/*!
 * Changes speed and plays a tone when in a certain range from an obstacle.
 */
class ChangeSpeed{
  //Percentage of speed the motors should run when in slow range:
  float speedPercentage;
  //Distance needed for the slow to take effect:
  int changeSpeedDistance;
  //End distance of the slow effect:
  int endChangeSpeedDistance;
  //Time that has to pass for the motors to be slow again by the same slow:
  int resetTime;
  //Stores if the motors are ready to run:
  boolean disableChangeSpeed;
  //Stores when them speed was last modified:
  unsigned long previousMillis;

  //Tone variables:
  //Tone object takes vars: toneAC( frequency [, volume [, length [, background ]]] ).
  int toneFrequency;
  int toneVolume;
  int toneLength;

  // Constructor - creates a ChangeSpeed object and initializes the member variables and state.

  public:
    /*!
     * Class constructor; creates a ChangeSpeed object and initializes the member variables and state
     * @param changeSpeedDis   Start of the distance at which you want the motors to change speed and play tone.
     * @param endChangeSpeedDis End of the distance at which you want the motors to change speed and play tone.
     * @param toneFreq          The frequency you want the tone should play.
     * @param speedPer          The speed the motors will run.
     */
    ChangeSpeed(int changeSpeedDis, int endChangeSpeedDis, int toneFreq, float speedPer)
      : speedPercentage(speedPer),
        changeSpeedDistance(changeSpeedDis),
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
  *    If there is: slows motors down to speedPercentage, plays tones (at: toneFrequency, toneFrequency and toneLength) and set a timer which waits 5s && for obstacles to be out of slow range.
  *    If there isn`t: if the last speed change occured more than 5s ago it resets the timer and a new slow can occur again.
  */
  void CheckForObstacle(){
    unsigned long currentMillis = millis();
    //opticky zmensit if, rozepsat, odentrovat, komentar:
    
    //Check and note if any sonar found an obstacle in the given range.
    boolean inChangeSpeedRange[NUMBER_OF_SONARS] = {
      //Left sonar in changeSpeed range:
      (0 < distance[0] && distance[0] <= changeSpeedDistance && endChangeSpeedDistance < distance[0]), 
      //Middle sonar in changeSpeed range:
      (0 < distance[1] && distance[1] <= changeSpeedDistance && endChangeSpeedDistance < distance[1]),
      //Right sonar in changeSpeed range:
      (0 < distance[2] && distance[2] <= changeSpeedDistance && endChangeSpeedDistance < distance[2])
    };

    //If any of the sonars found an obstacle dive into if loop.
    if(inChangeSpeedRange[0] || inChangeSpeedRange[1] || inChangeSpeedRange[2]){
      //Play tone.
      toneAC(toneFrequency, toneFrequency, toneLength);
      //If the speed hasn`t been changed in resetTime (5s) dive into if loop.
      if(disableChangeSpeed == false){
        runMotors(speedPercentage);
        disableChangeSpeed = true;
        previousMillis = currentMillis;
        log("EVENT: Change speed");
      }
      //If sonars didn`t find an obstacle and resetTime (5s) has passed, speed can be changed again.
    } else if(currentMillis - previousMillis >= resetTime){
      disableChangeSpeed = false;
    }
  }
};

//Make ChangeSpeed objects.
ChangeSpeed slow(35, 20, 3000, 0.5);
ChangeSpeed stop(20, 0, 3500, 0.0);

/**
 * Code that runs once after the device has been turned ON or connected to power.
 */
void setup() {
  //Startup.
  Serial.begin(9600);
  AFMS.begin();
  irrecv.enableIRIn();
}
/**
 * Code that runs in a neverending (until turned OFF, disconnected from power or out of power) loop.
 */
void loop() {

  //Dives into for loop if the ir reciever caught a new signal.
  if (irrecv.decode(&results)) {
    //Check if the signal corresponds with any action.
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
    //Runs motors at 100% (without slow).
    runMotors(1.0);

    //Reset IR reciever to wait for a new signal.
    irrecv.resume();
  }

  //Update distance array.
  updateDistance();

  //Slows motor if within slow range.
  slow.CheckForObstacle();
  stop.CheckForObstacle();
}
