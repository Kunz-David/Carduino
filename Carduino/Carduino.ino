/*!
 * @file Carduino.ino
 * @Author David Kunz
 * @date March, 2017
 * @brief 2WD DC Arduino car with obstacle detection.
 * @details Small three-wheeled car with 2 DC electromotors powering the front wheels and a single revolving wheel in the back.
 * The car is built around the Arduino platform and uses a set of three ultrasonic sensors to detect obstacles.
 */
/*!
 *  @defgroup pinLayout Pin Layout
 *  @brief Layout of all pins used by name in the @ref Carduino.ino.
 */

//Include used libraries:
//IR reciever library (https://github.com/z3t0/Arduino-IRremote) for more information see (http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html).
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
 * @brief Stream extension to create a Serial that can use printf. (Arduino does not have internal support for printf.)
 */
StreamEx mySerial = Serial;
/*!
 * @brief Turn logging ON/OFF
 * @details If TRUE logging is turned ON, if FALSE logging is OFF. This doesn`t effect the dedicated log button on the remote control.
 */
boolean loggingOn = true;

//Ultrasonic/Sonar variables:
/*!
 * @def NUMBER_OF_SONARS
 * @brief Number of ultrasonic sensors connected to the Arduino board.
 */
#define NUMBER_OF_SONARS 3
/*!
 * @def MAX_MEASURE_DISTANCE
 * @brief Maximum distance to which the ultrasonic sensors will measure.
 * @details Do not go over 200cm without increasing the delay time in the updateDistance() function. Increasing MAX_MEASURE_DISTANCE without increasing the delay will mean that the sensors don`t have enough time to get a returning signal and the output distances will be incorrect.
 */
#define MAX_MEASURE_DISTANCE 200 // Maximum distance (in cm) to ping.

/*!
 * @defgroup sonarPins Sonar Pins
 * @ingroup pinLayout
 * @brief Ultrasonic sensor logic pin layout.
 */ 
//@{
/*!Set trigger pin number for left ultrasonic sensor.*/
#define TRIG_PIN_LEFT_SONAR 35
/*!Set echo pin number for left ultrasonic sensor.*/
#define ECHO_PIN_LEFT_SONAR 33
/*!Set trigger pin number for middle ultrasonic sensor.*/
#define TRIG_PIN_MIDDLE_SONAR 41
/*!Set echo pin number for middle ultrasonic sensor.*/
#define ECHO_PIN_MIDDLE_SONAR 39
/*!Set tigger pin number for right ultrasonic sensor.*/
#define TRIG_PIN_RIGHT_SONAR 47
/*!Set echo pin number for right ultrasonic sensor.*/
#define ECHO_PIN_RINGT_SONAR 45
//@}
/*!
 * @brief Initializes a NewPing sensor object array with 3 ultrasonic sensors.
 * @details Constructor: NewPing sonar(trigger_pin, echo_pin [, max_cm_distance] = 500)
 */
NewPing sonar[NUMBER_OF_SONARS] = {
  NewPing(TRIG_PIN_LEFT_SONAR, ECHO_PIN_LEFT_SONAR, MAX_MEASURE_DISTANCE), //left sonar
  NewPing(TRIG_PIN_MIDDLE_SONAR, ECHO_PIN_MIDDLE_SONAR, MAX_MEASURE_DISTANCE), // middle sonar
  NewPing(TRIG_PIN_RIGHT_SONAR, ECHO_PIN_RINGT_SONAR, MAX_MEASURE_DISTANCE)  //right sonar
};
/*!
 * @brief Define a integer array where the distances measured by the ultrasonic sensors will be stored.
 * @sa updateDistance()
 */
int distance[NUMBER_OF_SONARS];

//Motorshield:
/*!
 * @brief Create the motor shield object with the default I2C address.
 */
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
/*!
 * @brief Select the motorshield port 1 for left motor.
 */
Adafruit_DCMotor *motorL = AFMS.getMotor(1);
/*!
 * @brief Select the motorshield port 2 for right motor.
 */
Adafruit_DCMotor *motorR = AFMS.getMotor(2);

//Motor variables:
/*!
 * @brief The coefficient of speed for the right wheel.
 * @details Used as a percentage of movement speed to the right wheel.
 * This is used as a method of steering when speedLeftCoefficient and speedRightCoefficient have different values.
 * Values: 0.0(no movement)-1.0(full speed)
 */
float speedRightCoefficient = 0.0;
/*!
 * @brief The coefficient of speed for the left wheel.
 * @details Used as a percentage of movement speed to the left wheel.
 * This is used as a method of steering when speedLeftCoefficient and speedRightCoefficient have different values.
 * Values: 0.0(no movement)-1.0(full speed)
 * @sa setMovementVars(float spdL, float spdR, uint8_t direcL, uint8_t direcR)
 * @sa runMotors(float speedPercentage)
 */
float speedLeftCoefficient = 0.0;
/*!
 * @brief Sets direction of the right motor.
 * @details Values: FORWARD = 1, BACKWARD = 2(, BRAKE = 3), RELEASE = 4
 */
uint8_t directionRight = RELEASE;
/*!
 * @brief Sets direction of the left motor.
 * @details Values: FORWARD = 1, BACKWARD = 2(, BRAKE = 3), RELEASE = 4
 */
uint8_t directionLeft = RELEASE;

//IR reciever:
/*!
 * @def RECEIVER_PIN
 * @brief IR Reciever logic pin.
 * @ingroup pinLayout
 */
#define RECEIVER_PIN 24
/*!
 * @brief Set a reciever pin.
 */
IRrecv irrecv(RECEIVER_PIN);
/*!
 * @brief Correct results from the IR reciever are stored.
 */
decode_results results;

//Initial gear speed:
int motorSpeed = 150;
/*!
 * Set speed coefficients and direction for both motors.
 * @param speedL   Speed left coefficient; percentage of power given to left motor (values 0.0-1.0).
 * @param speedR   Speed right coefficient; percentage of power given to left motor (values 0.0-1.0).
 * @param directionL Left motor direction; which direrection should the left motor run (values: FORWARD (=1), BACKWARD (=2), RELEASE (=4))(note: is relative to the wiring).
 * @param directionR Right motor direction; which direrection should the right motor run (values: FORWARD (=1), BACKWARD (=2), RELEASE (=4))(note: is relative to the wiring).
 */
void setMovementVars(float speedL, float speedR, uint8_t directionL, uint8_t directionR){
  speedLeftCoefficient = speedL;
  speedRightCoefficient = speedR;
  directionLeft = directionL;
  directionRight = directionR;
}

/*!
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

/*!
 * @brief Saves current distances found by each sonar to the distance array.
 * @remark Takes 3x15ms to execute (takes 11,8ms to get ultrasonic output).
 */
void updateDistance(){
  for (uint8_t i = 0; i < NUMBER_OF_SONARS; i++){
    distance[i] = sonar[i].ping_cm();
    delay(15); //it takes 11,8ms for signal that bounces off an obstacle 200cm away to return
  }
}

/*!
 * @brief Runs the motors at the given total speed percentage.
 * @param speedPercentage Percentage of total power to both motors. Values: 0.0-1.0.
 */
void runMotors(float speedPercentage){
  motorR->setSpeed(speedRightCoefficient*motorSpeed*speedPercentage);
  motorL->setSpeed(speedLeftCoefficient*motorSpeed*speedPercentage);
  motorR->run(directionRight);
  motorL->run(directionLeft);
}

/*!
 * @brief Changes speed and plays a tone when in a certain distance range from an obstacle.
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
  /*!
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
/*!
 * @brief Create a ChangeSpeed slow.
 * @details Create a ChangeSpeed object that slows the car to 50% on entering the range 35-20cm from the car and plays a tone at 3000Hz.
 */
ChangeSpeed slow(35, 20, 3000, 0.5);
/*!
 * @brief Create a ChangeSpeed stop.
 * @details Create a ChangeSpeed object that stops the car on entering the range 20-0cm from the car and plays a tone at 3500Hz.
 */
ChangeSpeed stop(20, 0, 3500, 0.0);

/*!
 * @brief Code that runs once after the device has been turned ON or connected to power.
 */
void setup() {

  // Opens serial port and sets data rate (baud) to 9600 bps.
  Serial.begin(9600);
  
  // Adafruit_MotorShield initialization. (Set PWM frequency and begin Wire communication.)
  AFMS.begin();
  
  // IRremote class initialization. (Sets pinMode, enables interrupts and enables the timer.)
  irrecv.enableIRIn();
}
/*!
 * @brief Code that runs in a never-ending (until turned OFF, disconnected from power or out of power) loop.
 */
void loop() {

  //Dives into for loop if the IR reciever caught a new signal.
  if (irrecv.decode(&results)) {
    //Check if the signal corresponds with any mapped action.
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