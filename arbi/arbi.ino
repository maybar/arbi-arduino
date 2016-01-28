/** \file arbi.ino
 * Robot control using the ISM radio control.
 * 
 * This program runs in the rover ARBI1.
 * Miguel A. 26 March 2015
 */

/*! \mainpage A Brief Description.
\section my-intro Introduction
This is the complete documentation of the source code for the application running in the rover ARBI.
It includes only the application but not the following libraries:
 - Adafruit_Motoshield
 - ServoTimer2
 - RobotMotor
 
The documentation of these libraries are included in the package for download.

\image html http://maybar.github.io/arbi-arduino/images/IMG_20150304_210451_560x640.jpg
\htmlinclude about-MyProject.html

*/
 
#include <SPI.h>
#include <Mirf.h>
//#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#ifdef ENABLE_IR
#include <IRremote.h>
#endif
#include <ServoTimer2.h> 
#include "RobotConstants.h"
#include <RobotMotor.h>  // 4wd motor library

#include <TimerOne.h>

//#define ENABLE_IR                 

unsigned long timer = 0;   //! 
bool b_no_move = false;    //! It indicates if the servo has to move 
bool b_move_sensor = false; 
ServoTimer2 myservo;  // create servo object to control a servo 

#ifdef ENABLE_IR
IRrecv irrecv(RECV_PIN);
#endif

int arp=0;
int pwm; 
int command;
bool b_pulsacion_larga =false;
unsigned long iKey = 0;
int local_power = NO_DATA;
bool b_cmd_tx= false;
int modo = MODO_MANUAL;
int time_batery_check=0;
int left_move_sonar =0;
int right_move_sonar =0;

#define RADIO_DATA_LEN 4
byte radio_tx_buffer[RADIO_DATA_LEN];
byte radio_rx_buffer[RADIO_DATA_LEN];


void RobotBegin();  //it is not in the original header

/** \fn void setup
	It initializes the variables and configures the hardware
*/
void setup()
{
  Serial.begin(9600);
  Serial.println("ARBI 1");
  dataDisplayBegin(DATA_nbrItems, labels, minRange, maxRange );
  blinkNumber(8); // open port while flashing. Needed for Leonardo only  
  pinMode(SONAR1_TRIG_PIN, OUTPUT); 
  pinMode(SONAR1_ECHO_PIN, INPUT);
  pinMode(SONAR2_TRIG_PIN, OUTPUT); 
  pinMode(SONAR2_ECHO_PIN, INPUT); 
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object 
  RobotBegin();  // create with the default frequency 1.6KHz
  
  moveSetSpeed(MIN_SPEED);
  setServoRange();
  moveStop();
#ifdef ENABLE_IR  
  irrecv.enableIRIn(); // Start the receiver
#endif
  command = CMD_STOP;
  StopServo();
  b_move_sensor=false;
  
   // * Setup pins / SPI.
  Mirf.cePin = 6;
  Mirf.csnPin = 7;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  
  Mirf.setRADDR((byte *)"arbi1");   // Configure receiving address.
  Mirf.payload = RADIO_DATA_LEN;   // Set the payload length 
  
   //* Write channel and payload config then power up receiver.
  Mirf.channel = 10;
  Mirf.config();
  
  Timer1.initialize(500000); // set a timer of length 500 msecond 
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

   //Configura la referencia del conversor analogico para el sensor de voltaje  
  analogReference(INTERNAL);      // set reference to internal (1.1V)
  analogRead(BATTERY_MONITOR_PIN);  // allow the ADC to settle
  delay(10);
}


void setServoRange()
{
   int index = (moveGetSpeed() - MIN_SPEED) / SPEED_TABLE_INTERVAL;
   int check_range_sonar = 300*50/minDistance[index];
   left_move_sonar =  MID_MOVE_SONAR +  check_range_sonar/2 ; 
   right_move_sonar = MID_MOVE_SONAR -  check_range_sonar/2 ;   

}

/// --------------------------
/// Custom ISR Timer Routine
/// --------------------------
void timerIsr()
{
   b_cmd_tx = true;
   time_batery_check++;
}

/** \fn int incPulse
	It increments the argument. 	 
 	 This function just increments a value until it reaches a maximum
*/
int incPulse(int val){
static bool b_inc =false; 
  if( val  >= left_move_sonar ){
    b_inc = false;
  }
  if(val <= right_move_sonar){
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
  checkMovement(modo); 
  processRadio();   
  processCommand();
#ifdef ENABLE_IR
  //process_IR();
#endif
  MoveServo();
  batteryCheck(local_power);
}

void processRadio()
{
   static int i_cmd_send=0;    //rx
   
   switch(i_cmd_send)
   {
      case 0:  //rx
      {
         if (Mirf.dataReady())
         {           
            Mirf.getData((byte *) &radio_rx_buffer[0]);
            if ((radio_rx_buffer[0] != modo) ||(radio_rx_buffer[1]!=command)) {
               modo = radio_rx_buffer[0];    //mode
               command = radio_rx_buffer[1]; //comando en modo manual
               i_cmd_send=1;  //envia la respuesta
            }
         }
         if (b_cmd_tx)  {  //solicita el envío
            i_cmd_send=1;
            b_cmd_tx = false;
         }
         break;
      }
      case 1:  //tx
      {
         //Serial.println("TX");
         Mirf.setTADDR((byte *)"arco");
         radio_tx_buffer[0] = modo+command; //checksum de comando
         radio_tx_buffer[1] = local_power;
         radio_tx_buffer[2] = moveGetSpeed();
         radio_tx_buffer[3] = arp;
         Mirf.send((byte *)&radio_tx_buffer[0]);
         i_cmd_send = 2;
         break;
      }
      case 2:  //wait to send
      {
         if (!Mirf.isSending())
         {
            i_cmd_send = 0;
         }
         break;
      }
      default:
      {
         break;
      }
   }  
}

/** \fn void processCommand
// function to execute the commands of movement.
// This commends can arrive from IR receiver, or the ISM radio or the autonomous internal control.
*/
void processCommand()
{
   static int old_command=CMD_NONE;
   if (command == old_command) return;
   old_command = command;
  switch(command)
  {
    case CMD_STOP:
    {
      moveStop();
      StopServo();
      break;
    }
    case CMD_BW:
    {    
      arp=0;
      moveBackward();
      StopServo();
      break;
    }   
    case CMD_FW:
    {
       arp=0;
      moveForward();   
      b_move_sensor = true;    
      break;
    }  
    case CMD_FW_LEFT:    
    {
       arp=0;
      moveLeft(); 
      break; 
    }    
    case CMD_FW_RIGHT:    
    {
       arp=0;
      moveRight();     
      break;
    } 
    case CMD_BW_LEFT:    
    {
       arp=0;
      //compensateSpeed();
      //motorStop(MOTOR_LEFT);
      //motorReverse(MOTOR_RIGHT, speed);
      //StopServo();
      break;
    }    
    case CMD_BW_RIGHT:    
    {
       arp=0;
      /*compensateSpeed();
      motorStop(MOTOR_RIGHT);
      motorReverse(MOTOR_LEFT, speed);  
      StopServo(); */   
      break;
    } 
    case CMD_ROT_LEFT:    
    {
       arp=0;
      moveStartRotation(DIR_LEFT);
      break; 
    }    
    case CMD_ROT_RIGHT:    
    {
       arp=0;
      moveStartRotation(DIR_RIGHT);   
      break;
    } 
    case CMD_DECREASE:    
    {
      moveSlower(10);  
      setServoRange();  
      break;
    }    
    case CMD_INCREASE:    
    { 
      moveFaster(10);
      setServoRange();
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



