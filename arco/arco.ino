/** \file arco.ino
 * Remote Robot control for ARBI-Arduino.
 * 
 * This program runs in the ARCO hardware.
 * Miguel A. 17 May 2015
 */

/*! \mainpage A Brief Description.
\section my-intro Introduction
This is the complete documentation of the source code for the application running in the remote ARCO.
Radios:
XD_FST 433MHz
nRF24L01 868MHz

\image html http://maybar.github.io/arbi-arduino/images/IMG_20150304_210451_560x640.jpg
\htmlinclude about-MyProject.html

*/
 

#include <SPI.h>
#include <Mirf.h>
//#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "icons.h"

#include <TimerOne.h>

Adafruit_SSD1306 display(-1);  //No tiene pin de reset

#define LED_PIN 13
#define PIN_JOY_LEFT_X   0  //Axe X of left Joystick connected to analog pin 0
#define PIN_JOY_LEFT_Y   1  //Axe Y of left Joystick connected to analog pin 1
#define PIN_JOY_LEFT_SW   10 // Sw of right Joystick connected to digital pin 10
#define PIN_JOY_RIGHT_X   2  //Axe X of right Joystick connected to analog pin 0
#define PIN_JOY_RIGHT_Y   3  //Axe Y of right Joystick connected to analog pin 1
#define PIN_JOY_RIGHT_SW   9 // Sw of right Joystick connected to digital pin 9
#define PIN_ENCODER_A   5  // A pin of the encoder connected to digital pin 5
#define PIN_ENCODER_B   4  // A pin of the encoder connected to digital pin 4
#define PIN_ENCODER_SW   8  // SW of the encoder connected to digital pin 8

#define CMD_NONE  0
#define CMD_STOP  1
#define CMD_FW    2
#define CMD_BW    3
#define CMD_FW_LEFT     4
#define CMD_FW_RIGHT    5
#define CMD_BW_LEFT     6
#define CMD_BW_RIGHT    7
#define CMD_DECREASE    8
#define CMD_INCREASE    9
#define CMD_ROT_LEFT    10
#define CMD_ROT_RIGHT    11

#define MODO_MANUAL  0
#define MODO_AUTO  1


struct SJoystick
{
   int i_x;   
   int i_y;
   bool b_sw;
};


#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define RADIO_DATA_LEN 4
byte radio_tx_buffer[RADIO_DATA_LEN];
byte radio_rx_buffer[RADIO_DATA_LEN];

int local_power = NO_DATA;
int remote_power = NO_DATA;
int paint_tx = 0;
//int paint_rx = 0;
int timeout_rx = 0;     //contador de fallos de recepcion
int time_batery_check=300;
byte command = CMD_NONE;
bool b_tx_flag = false;
int remote_speed =0;
int modo = MODO_MANUAL;
int remote_arp = 0;

/** \fn void setup
	It initializes the variables and configures the hardware
*/
void setup()
{
  pinMode(PIN_JOY_LEFT_SW, INPUT);             
  pinMode(PIN_JOY_RIGHT_SW, INPUT);  
  digitalWrite(PIN_JOY_LEFT_SW, HIGH); //turn pullup resistor on
  digitalWrite(PIN_JOY_RIGHT_SW, HIGH); //turn pullup resistor on 
  pinMode(PIN_ENCODER_A, INPUT);             
  pinMode(PIN_ENCODER_B, INPUT); 
  pinMode(PIN_ENCODER_SW, INPUT);  
  digitalWrite(PIN_ENCODER_A, HIGH); //turn pullup resistor on
  digitalWrite(PIN_ENCODER_B, HIGH); //turn pullup resistor on 
  digitalWrite(PIN_ENCODER_SW, HIGH); //turn pullup resistor on 
  
  //Display -----------------------------------------------------
  display.begin();
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(40,10);
  display.println("ARCO");
  display.println("  Arduino");
  display.display();
  delay(2000);
    //Valores por defecto
  ShowDefaultWindow();
  //---------------------------------------------------------------
 
  Serial.begin(9600);
  Serial.println("ARCO-Arduino");

   // * Setup pins / SPI.
  Mirf.cePin = 6;
  Mirf.csnPin = 7;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  
  Mirf.setRADDR((byte *)"arco");   // Configure receiving address.
  Mirf.payload = RADIO_DATA_LEN;   // Set the payload length 
  
   //* Write channel and payload config then power up receiver.
  Mirf.channel = 10;
  Mirf.config();
  

  
  Timer1.initialize(500000); // set a timer of length 200 msecond 
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
}

void ShowDefaultWindow()
{
  local_power = NO_DATA;
  remote_power = NO_DATA; 
  paint_tx = 0;
  timeout_rx = 0;
  remote_speed =0;
  modo = MODO_MANUAL;
  remote_arp = 0;
  time_batery_check=300;
  display.clearDisplay();
  display.drawBitmap(0, 0, &logo_bat[NO_DATA], ICON_WIDTH, ICON_HEIGHT, WHITE);
  display.drawBitmap(ICON_WIDTH+5, 0, &logo_bat[NO_DATA], ICON_WIDTH, ICON_HEIGHT, WHITE);
  display.display();
  display.setTextSize(1);
  display.drawRect(121, 0, 7, 64, WHITE);
}

/// --------------------------
/// Custom ISR Timer Routine
/// --------------------------
void timerIsr()
{
   if (timeout_rx < 13) timeout_rx++;
   time_batery_check++;
   //if (paint_rx>0) paint_rx--;
   if (paint_tx>0) paint_tx--;

}

void loop()
{
   static bool flagMove = false;
   static byte old_command = CMD_NONE;
   struct SJoystick s_joystick_left;
   struct SJoystick s_joystick_right;
   static int right_cmd=0;
   static bool set_speed=false;
   processJoystick(s_joystick_left, s_joystick_right);

   if (s_joystick_left.i_y > 0) 
   {
      command = CMD_FW;
      if (s_joystick_left.i_x > 0) command = CMD_FW_RIGHT;
      else if (s_joystick_left.i_x < 0) command = CMD_FW_LEFT;
   }
   else if (s_joystick_left.i_y < 0) {
      command = CMD_BW;
      if (s_joystick_left.i_x > 0) command = CMD_BW_RIGHT;
      else if (s_joystick_left.i_x < 0) command = CMD_BW_LEFT;
   }
   else {
      if (s_joystick_left.i_x > 0) command = CMD_ROT_RIGHT;
      else if (s_joystick_left.i_x < 0) command = CMD_ROT_LEFT;
      else {
         if (flagMove==true) {
            command = CMD_STOP;
            flagMove = false;
            }
         }
   }
   //Joystick derecho
   if (s_joystick_right.i_y > 0){
      if(right_cmd ==0){
       right_cmd=1;
       command = CMD_INCREASE;
       b_tx_flag = true;
       }
   }
   else if (s_joystick_right.i_y < 0){
      if(right_cmd ==0){
       right_cmd=1;
       command = CMD_DECREASE;
       b_tx_flag = true;
       }
    }
   else {
      if(right_cmd ==1){
         command = CMD_NONE;
         right_cmd=0;
         b_tx_flag = true;
      }
   }
   
   if ((command == CMD_FW) || (command == CMD_BW) || (command == CMD_FW_LEFT) ||(command == CMD_FW_RIGHT) ||
   (command == CMD_BW_LEFT) ||(command == CMD_BW_RIGHT) ||(command == CMD_ROT_LEFT) ||(command == CMD_ROT_RIGHT))
      flagMove =true;
      
   if (readSwEncoder() == true){
      processConfig();
   }
   
   if (old_command != command){
      /*if(command != CMD_NONE) */b_tx_flag = true;
      old_command = command;
   }
   processRadio();
   batteryCheck(local_power);
   processDisplay();
   
    //delay(500);
}

void processConfig()
{
   //Timer1.stop();
   Serial.println("init processConfig");
   display.clearDisplay();
   display.display();
   delay(250);
   while(readSwEncoder()== false)
   {
   }
   Serial.println("close processConfig");
   ShowDefaultWindow();
   processDisplay();
   //Timer1.start();
}

void processRadio(void)
{
#define RADIO_RX  0
#define RADIO_TX  1
#define RADIO_WAIT 2

   static int i_sm_radio=RADIO_RX;    //rx
   static int checksum=0;

   switch(i_sm_radio)
   {
      case RADIO_RX:  //rx
      {
         if (Mirf.dataReady())
         {
            //Serial.println("RX");
            Mirf.getData((byte *) &radio_rx_buffer[0]);
            checksum= radio_rx_buffer[0];
            remote_power = radio_rx_buffer[1];
            remote_speed = radio_rx_buffer[2];
            remote_arp   = radio_rx_buffer[3];
            timeout_rx=0;
            //paint_rx = 1;  //1x200ms
            if((checksum != (modo+command)) && (remote_arp==0)) b_tx_flag = true;
            //Serial.println(remote_power);
         }

         if (timeout_rx > 10)  {
            remote_power=NO_DATA;   //100*200ms =2 segundos
            remote_speed = 255;
         }
         if (b_tx_flag == true) i_sm_radio = RADIO_TX;
         break;
      }
      case RADIO_TX:  //tx
      {
         if(timeout_rx==13) break;
         Mirf.setTADDR((byte *)"arbi1");
         radio_tx_buffer[0] = modo;
         radio_tx_buffer[1] = command;
         Mirf.send((byte *)&radio_tx_buffer[0]);
         i_sm_radio = RADIO_WAIT;   //wait to send
         paint_tx = 1;  //pinta el icono 1x200ms
         b_tx_flag = false;   //comando ejecutado
         
          Serial.print("TX:");
          Serial.println(command);
         break;
      }
      case RADIO_WAIT:  //wait to send
      {
         if (!Mirf.isSending())
         {
            i_sm_radio = RADIO_RX;
         }
         break;
      }
      default:
      {
         break;
      }
   }  
   
}

void processJoystick(struct SJoystick &rs_joystick_left, 
   struct SJoystick &rs_joystick_right)
{
   int ivrx = 0;                  // variable to read the value from the analog pin
   int ivry = 0;                  // variable to read the value from the analog pin
   int i_current_sw;   
   static volatile int i_lastSW_left = 1;
   static volatile int i_lastSW_right = 1;
   // reads the value of the variable resistor 
   delay(10);
   ivrx = analogRead(PIN_JOY_LEFT_X); 
   delay(10);
   ivry = analogRead(PIN_JOY_LEFT_Y);

   rs_joystick_left.i_x = treatValue(ivrx);
   rs_joystick_left.i_y = -1*treatValue(ivry);
   rs_joystick_left.b_sw = false;
   i_current_sw = digitalRead(PIN_JOY_LEFT_SW);
   if (i_current_sw == 0)
   { 
      if(i_lastSW_left==1) rs_joystick_left.b_sw = true;
   }
   i_lastSW_left = i_current_sw ;
   
   delay(10);
   ivrx = analogRead(PIN_JOY_RIGHT_X); 
   delay(10);
   ivry = analogRead(PIN_JOY_RIGHT_Y); 
   rs_joystick_right.i_x = -1*treatValue(ivrx);
   rs_joystick_right.i_y = treatValue(ivry);
   rs_joystick_right.b_sw = false;
   i_current_sw = digitalRead(PIN_JOY_RIGHT_SW);
   if (i_current_sw == 0)
   { 
      if(i_lastSW_right==1) rs_joystick_right.b_sw = true;
   }
   i_lastSW_right = i_current_sw ;
}


int treatValue(int data) {
  return (data * 11 / 1024) - 5;
}

/** \fn void readEncoder
// function to read the encoder
*/ 
int readEncoder(void)
 {

   static volatile int lastEncoded = 0;
   static volatile long encoderValue = 0;
   static long lastencoderValue = 0;

   int MSB = digitalRead(PIN_ENCODER_A); //MSB = most significant bit
   int LSB = digitalRead(PIN_ENCODER_B); //LSB = least significant bit

   int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
   int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

   if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
   if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

   lastEncoded = encoded; //store this value for next time

   int i_state = 0;
   if (encoderValue > lastencoderValue) i_state = 1;
   if (encoderValue < lastencoderValue) i_state = -1;
   
   lastencoderValue = encoderValue;
   return i_state;
 }
 
 bool readSwEncoder()
 {
   static volatile int i_lastSW = 1;
   bool b_sw = false;
   int i_current_sw =digitalRead(PIN_ENCODER_SW);
   if (i_current_sw == 0)
   { 
      if(i_lastSW==1) b_sw = true;
   }
   i_lastSW = i_current_sw;
   return b_sw;
 }
 
 void processDisplay()
 {
static int old_local_power = NO_DATA;
static int old_remote_power = NO_DATA;
static int old_paint_tx = 0;
static int old_timeout_rx = 255;
static int old_remote_speed =0;
static int old_modo =255;
static int old_remote_arp =0;

   if(old_local_power != local_power){    
       //Dibujar el indicador de bateria
       display.fillRect(0, 0, ICON_WIDTH, ICON_HEIGHT, BLACK);
       display.drawBitmap(0, 0, &logo_bat[local_power*48], ICON_WIDTH, ICON_HEIGHT, WHITE);      
       old_local_power = local_power;
    }
    if (old_remote_power != remote_power){
       display.fillRect(30, 0, ICON_WIDTH, ICON_HEIGHT, BLACK);
       display.drawBitmap(30, 0, &logo_bat[remote_power*48], ICON_WIDTH, ICON_HEIGHT, WHITE);
       old_remote_power = remote_power;
    }
    if (old_paint_tx != paint_tx) {
       if (paint_tx==0) display.fillRect(90, 0, ICON_WIDTH, ICON_HEIGHT, BLACK);
       else if (old_paint_tx==0){
          display.setCursor(90,0);
          display.setTextSize(1);
          display.println("TX");
       }
       
       old_paint_tx = paint_tx;
    }
    // Barra de error 
    if (old_timeout_rx != timeout_rx){
       Serial.println(timeout_rx);
       if(timeout_rx%6==0){
                 
         display.fillRect(123, 3, 3, 60, BLACK);
         int pos_y= 6*timeout_rx;
         int len_y= 60-pos_y;
         display.fillRect(123, pos_y+2, 3, len_y, WHITE);
         old_timeout_rx = timeout_rx;
         Serial.println(timeout_rx);
      }    
    }
    // Dibuja el mode
    if (old_modo != modo){
       display.setTextSize(1);
       display.fillRect(80, 15, 30, 10, BLACK);
       display.setCursor(80,15);
       if(modo==MODO_MANUAL) display.println("MANUAL");
       else if(modo==MODO_AUTO) display.println("AUTO");
       old_modo = modo;
    }    
    // Dibuja la velocidad
    if (old_remote_speed != remote_speed){
       display.fillRect(30, 25, 60, 21, BLACK);
       display.setTextSize(3);
       display.setCursor(30,25);
       if(remote_speed==255)display.println("---");
       else display.println(remote_speed);
       old_remote_speed = remote_speed;
    }
    //Dibuja ARP
    if (old_remote_arp != remote_arp){
       display.fillRect(30, 55, 30, 9, BLACK);
       display.setTextSize(1);
       display.setCursor(30,55);
       if(remote_arp==1) display.println("ARP");
       old_remote_arp = remote_arp;
    }
    
    display.display();
 }
 
/** \fn void blinkNumber
// function to indicate numbers by flashing the built-in LED
*/
/*void blinkNumber( byte number) {
   pinMode(LED_PIN, OUTPUT); // enable the LED pin for output
   while(number--) {
     digitalWrite(LED_PIN, HIGH); delay(100);
     digitalWrite(LED_PIN, LOW);  delay(400);
   }
}*/



