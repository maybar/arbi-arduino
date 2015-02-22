
/*******************************************
* Robot4wd_IR.ino
* Robot control using IR remote control 

* Josemi 11 January 2015
********************************************/

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <IRremote.h>
#include <ServoTimer2.h> 
#include "RobotConstants.h"
//#include <RobotMotor.h>  // 4wd motor library

const int LED_PIN = 13;
const int RECV_PIN = 8;

signed int speed = MIN_SPEED; // percent of maximum speed
int old_speed = MIN_SPEED;
unsigned long timer = 0;
bool b_no_move = false;
bool b_move_sensor = false; 
ServoTimer2 myservo;  // create servo object to control a servo 

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// Select which 'port' M1, M2, M3 or M4.
Adafruit_DCMotor *Motor_Left_Front = AFMS.getMotor(3);
Adafruit_DCMotor *Motor_Right_Front = AFMS.getMotor(2);
Adafruit_DCMotor *Motor_Left_Rear = AFMS.getMotor(4);
Adafruit_DCMotor *Motor_Right_Rear = AFMS.getMotor(1);


IRrecv irrecv(RECV_PIN);

int pwm; 
int command;
int right_run, left_run;
bool b_cambio = false;
bool b_pulsacion_larga =false;
unsigned long iKey = 0;



void setup()
{
  Serial.begin(9600);
  Serial.println("MotorTest4wd!");
  blinkNumber(8); // open port while flashing. Needed for Leonardo only  
   
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
  AFMS.begin();  // create with the default frequency 1.6KHz
     
  // scale percent into pwm range (0-255) 
  pwm= map(speed, 0,100, 0,255);  
  
  // Set the speed to start, from 0 (off) to 255 (max speed)
  Motor_Left_Front->setSpeed(pwm);
  Motor_Left_Front->run(RELEASE);// turn on motor
  Motor_Right_Front->setSpeed(pwm);
  Motor_Right_Front->run(RELEASE);// turn on motor
  Motor_Left_Rear->setSpeed(pwm);
  Motor_Left_Rear->run(RELEASE);// turn on motor
  Motor_Right_Rear->setSpeed(pwm);
  Motor_Right_Rear->run(RELEASE);// turn on motor
  
  irrecv.enableIRIn(); // Start the receiver
  command = CMD_STOP;
  right_run = RELEASE;
  left_run = RELEASE;
  StopServo();
  b_move_sensor=true;
}

// this function just increments a value until it reaches a maximum

int incPulse(int val){
static bool b_inc =false; 
  if( val  >= MAX_MOVE_SONAR ){
    b_inc = false;
  }
  if(val <= MIN_MOVE_SONAR){
    b_inc = true;
  }
  if (b_inc == true)
    val = val + INC_MOVE_SONAR;
  else
    val = val - INC_MOVE_SONAR;
  return val;
}

void MoveServo()
{
  if (b_move_sensor == true) {
    int val = incPulse( myservo.read());
    myservo.write(val);
    delay(10);
  }
}
void StopServo()
{
  myservo.write(MID_MOVE_SONAR);
  b_move_sensor=false;
}


// run over and over
void loop()
{  
  b_cambio = true;
  switch(command)
  {
    case CMD_STOP:
    {
      left_run = RELEASE;
      right_run = RELEASE;
      speed = old_speed;
      break;
    }
    case CMD_BW:
    {
      left_run = BACKWARD;
      right_run = BACKWARD;
      speed = old_speed;
      break;
    }   
    case CMD_FW:
    {
      left_run = FORWARD;
      right_run = FORWARD;
      speed = old_speed;;
      break;
    }  
    case CMD_FW_LEFT:    
    {
      left_run = RELEASE;
      right_run = FORWARD;
      old_speed = speed;
      speed += INC_SPEED;
      break; 
    }    
    case CMD_FW_RIGHT:    
    {
      left_run = FORWARD;
      right_run = RELEASE;
      old_speed = speed;
      speed += INC_SPEED;
      break;
    } 
    case CMD_BW_LEFT:    
    {
      left_run = RELEASE;
      right_run = BACKWARD;
      old_speed = speed;
      speed += INC_SPEED;
      break;
    }    
    case CMD_BW_RIGHT:    
    {
      left_run = BACKWARD;
      right_run = RELEASE;
      old_speed = speed;
      speed += INC_SPEED;
      break;
    } 
    case CMD_DECREASE:    
    {
      
      speed=speed - SPEED_TABLE_INTERVAL;
      break;
    }    
    case CMD_INCREASE:    
    { 
      speed+=SPEED_TABLE_INTERVAL;     
      break;
    }   
    case CMD_NONE:    
    {
      b_cambio = false;
      break;
    }   
default:
b_cambio = false;
break;
  }
  if (b_cambio == true)
  {
    Serial.println(command);
    if (speed < MIN_SPEED) speed = 0;
    else if (speed > 100) speed = 100; 
    if ((left_run == FORWARD) ||  (right_run == FORWARD)) b_move_sensor = true;
    else StopServo();
    
    pwm= map(speed, 0,100, 0,255);
    Motor_Left_Front->setSpeed(pwm);
    Motor_Left_Front->run(left_run);
    Motor_Left_Rear->setSpeed(pwm);
    Motor_Left_Rear->run(left_run);
    
    Motor_Right_Front->setSpeed(pwm);
    Motor_Right_Front->run(right_run);
    Motor_Right_Rear->setSpeed(pwm);
    Motor_Right_Rear->run(right_run);
  }
  process_IR();
  
  MoveServo();
}

// function to indicate numbers by flashing the built-in LED
void blinkNumber( byte number) {
   pinMode(LED_PIN, OUTPUT); // enable the LED pin for output
   while(number--) {
     digitalWrite(LED_PIN, HIGH); delay(100);
     digitalWrite(LED_PIN, LOW);  delay(400);
   }
}




