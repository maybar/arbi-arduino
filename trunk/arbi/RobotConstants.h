
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
#define RANGE_SONAR  1180
#define MAX_MOVE_SONAR  MID_MOVE_SONAR +  RANGE_SONAR/2  //2250
#define MIN_MOVE_SONAR  MID_MOVE_SONAR -  RANGE_SONAR/2    //750
#define INC_MOVE_SONAR  5

//const int MIN_SPEED = 60; // first table entry is 60% speed
//const int SPEED_TABLE_INTERVAL = 10; // each table entry is 10% faster speed
//const int NBR_SPEEDS =  1 + (100 - MIN_SPEED)/ SPEED_TABLE_INTERVAL;
 
//int speedTable[NBR_SPEEDS]  =  {60,   70,   80,   90,  100}; // speeds  
//int rotationTime[NBR_SPEEDS] = {5500, 3300, 2400, 2000, 1750}; // time
