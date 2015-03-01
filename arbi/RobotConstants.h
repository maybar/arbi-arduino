
#define CMD_NONE 0
#define CMD_STOP 1
#define CMD_FW  2
#define CMD_BW  3
#define CMD_FW_LEFT  4
#define CMD_FW_RIGHT  5
#define CMD_BW_LEFT  6
#define CMD_BW_RIGHT 7
#define CMD_DECREASE  8
#define CMD_INCREASE  9

#define INC_SPEED 20

#define KEY_2  16718055
#define KEY_4  16716015
#define KEY_5  16726215
#define KEY_6  16734885
#define KEY_MENOS  16754775
#define KEY_MAS  16748655
#define KEY_8  16730805

#define MID_MOVE_SONAR  1590
#define MAX_RANGE_SONAR  1180
#define CHECK_RANGE_SONAR  450
#define MAX_MOVE_SONAR  MID_MOVE_SONAR +  MAX_RANGE_SONAR/2  //2250
#define MIN_MOVE_SONAR  MID_MOVE_SONAR -  MAX_RANGE_SONAR/2    //750

#define LEFT_MOVE_SONAR  MID_MOVE_SONAR +  CHECK_RANGE_SONAR/2  //2250
#define RIGHT_MOVE_SONAR  MID_MOVE_SONAR -  CHECK_RANGE_SONAR/2    //750
#define INC_MOVE_SONAR  10

// defines for directions
const int DIR_LEFT   = 0;
const int DIR_RIGHT  = 1;
const int DIR_CENTER = 2; 

const int LED_PIN = 13;
const int RECV_PIN = 8;
const int SERVO_PIN = 9;  //servo de movimiento de SONAR1

#define SONAR1  0  // sensor ultrasonico delantero
#define SONAR2  1 // sensor ultrasonico trasero

const int SONAR1_TRIG_PIN = 5;  //Trigger pin of HC-SR04
const int SONAR1_ECHO_PIN = 4;  //Echo pin of HC-SR04
const int SONAR2_TRIG_PIN = 3;  //Trigger pin of HC-SR04
const int SONAR2_ECHO_PIN = 2;  //Echo pin of HC-SR04

const float Drag = 2.0; // coeficieente de rozamiento del suelo

enum {MOV_LEFT, MOV_RIGHT, MOV_FORWARD,
      MOV_BACK, MOV_ROTATE, MOV_STOP};
      
const char* states[] = {"Left", "Right", "Forward", 
                        "Back", "Rotate", "Stop"};
                        
const int  MIN_DISTANCE = 20;     // robot stops when object is nearer (in cm)
const int  CLEAR_DISTANCE = 60;  // distance in cm considered attractive to move

enum {DATA_start, DATA_LEFT, DATA_CENTER, DATA_RIGHT,DATA_DRIFT,DATA_DISTANCE,
     DATA_nbrItems};

char* labels[]= {"","Left Line","Center Line","Right Line","Drift","Distance"};
int minRange[]= { 0,          0,            0,           0,  -1023,        0 };  
int maxRange[]= { 0,       1023,         1023,        1023,   1023,       144};  

