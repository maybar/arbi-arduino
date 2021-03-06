/** \file Robot4wd_IR.ino
 * Robot control using IR remote control.
 * 
 * This program runs in the rover ARBI1.
 * Josemi 11 January 2015
 */
 
#ifdef ENABLE_IR
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <IRremote.h>
#include <ServoTimer2.h> 
#include "RobotConstants.h"
#include <RobotMotor.h>  // 4wd motor library


//signed int speed = MIN_SPEED; // percent of maximum speed                     

unsigned long timer = 0;   //! 
bool b_no_move = false;    //! It indicates if the servo has to move 
bool b_move_sensor = false; 
ServoTimer2 myservo;  // create servo object to control a servo 


IRrecv irrecv(RECV_PIN);

int pwm; 
int command;
bool b_pulsacion_larga =false;
unsigned long iKey = 0;

void RobotBegin();  //it is not in the original header

/** \fn void setup
	It initializes the variables and configures the hardware
*/
void setup()
{
  Serial.begin(9600);
  Serial.println("MotorTest4wd!");
  dataDisplayBegin(DATA_nbrItems, labels, minRange, maxRange );
  blinkNumber(8); // open port while flashing. Needed for Leonardo only  
  pinMode(SONAR1_TRIG_PIN, OUTPUT); 
  pinMode(SONAR1_ECHO_PIN, INPUT);
  pinMode(SONAR2_TRIG_PIN, OUTPUT); 
  pinMode(SONAR2_ECHO_PIN, INPUT); 
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object 
  RobotBegin();  // create with the default frequency 1.6KHz
  
  moveSetSpeed(MIN_SPEED);
  moveStop();
  
  irrecv.enableIRIn(); // Start the receiver
  command = CMD_STOP;
  StopServo();
  b_move_sensor=false;
  
  //test
  /*speed =70;
  moveRotate (360);
  delay(1000); */
}

/** \fn int incPulse
	It increments the argument. 	 
 	 This function just increments a value until it reaches a maximum
*/
int incPulse(int val){
static bool b_inc =false; 
  if( val  >= LEFT_MOVE_SONAR ){
    b_inc = false;
  }
  if(val <= RIGHT_MOVE_SONAR){
    b_inc = true;
  }
  if (b_inc == true)
    val = val + INC_MOVE_SONAR;
  else
    val = val - INC_MOVE_SONAR;
  return val;
}

/** \fn void MoveServo
	Move the SONAR1 servo 
 	This function moves the servo 1 point to left or right. When it reaches the maximum position, 
 	the servo moves in opposite direction.
*/
void MoveServo()
{
  if (b_move_sensor == true) {
    int val = incPulse( myservo.read());
    myservo.write(val);
    delay(10);
  }
}

/** \fn void StopServo
	Stop the servo of the SONAR1.
*/
void StopServo()
{
  myservo.write(MID_MOVE_SONAR);
  b_move_sensor=false;
}

/** \fn void loop
// run over and over
*/
void loop()
{  
  checkMovement();  
  processCommand();
  process_IR();
  MoveServo();
}

/** \fn void processCommand
// function to execute the commands of movement.
// This commends can arrive from IR receiver, or the ISM radio or the autonomous internal control.
*/
void processCommand()
{
  switch(command)
  {
    case CMD_STOP:
    {
      //speed = old_speed;
      moveStop();
      StopServo();
      break;
    }
    case CMD_BW:
    {
      //speed = old_speed;
      //moveSetSpeed(old_speed);     
      moveBackward();
      StopServo();
      break;
    }   
    case CMD_FW:
    {
      //speed = old_speed;
      //moveSetSpeed(old_speed);
      moveForward();   
      b_move_sensor = true;    
      break;
    }  
    case CMD_FW_LEFT:    
    {
      //compensateSpeed();
      //moveSetSpeed(speed);
      moveLeft(); 
      b_move_sensor = false;
      break; 
    }    
    case CMD_FW_RIGHT:    
    {
      //compensateSpeed();
      //moveSetSpeed(speed);
      moveRight();   
      b_move_sensor = false;   
      break;
    } 
    case CMD_BW_LEFT:    
    {
      //compensateSpeed();
      //motorStop(MOTOR_LEFT);
      //motorReverse(MOTOR_RIGHT, speed);
      //StopServo();
      break;
    }    
    case CMD_BW_RIGHT:    
    {
      /*compensateSpeed();
      motorStop(MOTOR_RIGHT);
      motorReverse(MOTOR_LEFT, speed);  
      StopServo(); */   
      break;
    } 
    case CMD_DECREASE:    
    {
      moveSlower(10);    
      break;
    }    
    case CMD_INCREASE:    
    { 
      moveFaster(10);
      break;
    }   
    case CMD_NONE:    
    {
      
      break;
    }   
default:

break;
  }
}

/** \fn void blinkNumber
// function to indicate numbers by flashing the built-in LED
*/
void blinkNumber( byte number) {
   pinMode(LED_PIN, OUTPUT); // enable the LED pin for output
   while(number--) {
     digitalWrite(LED_PIN, HIGH); delay(100);
     digitalWrite(LED_PIN, LOW);  delay(400);
   }
}
#endif

