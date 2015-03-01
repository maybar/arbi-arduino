/*************************************
 Drive: mid level movement functions
*************************************/

int moveState = MOV_STOP;   // what robot is doing
 
int  moveSpeed   = 0; // move speed stored here (0-100%)
int  speedIncrement = 10; // percent to increase or decrease speed

void moveBegin()
{
   motorBegin(MOTOR_LEFT); 
   motorBegin(MOTOR_RIGHT); 
   moveStop();
}

void moveLeft()
{
  motorForward(MOTOR_LEFT,  0);
  motorForward(MOTOR_RIGHT, moveSpeed);
  changeMoveState(MOV_LEFT);  
}

void moveRight()
{
  motorForward(MOTOR_LEFT,  moveSpeed);
  motorForward(MOTOR_RIGHT, 0);
  changeMoveState(MOV_RIGHT); 
}

void moveStop()
{
  motorStop(MOTOR_LEFT);
  motorStop(MOTOR_RIGHT);
  changeMoveState(MOV_STOP);  
}

void moveBrake()
{
  motorBrake(MOTOR_LEFT);
  motorBrake(MOTOR_RIGHT);
  changeMoveState(MOV_STOP);  
}

void moveBackward()
{
  motorReverse(MOTOR_LEFT, moveSpeed);
  motorReverse(MOTOR_RIGHT, moveSpeed);  
  changeMoveState(MOV_BACK);  
}

void moveForward()
{
  motorForward(MOTOR_LEFT,  moveSpeed);
  motorForward(MOTOR_RIGHT, moveSpeed);
  changeMoveState(MOV_FORWARD);  
}

void moveSetSpeed(int speed)
{
  motorSetSpeed(MOTOR_LEFT, speed) ;
  motorSetSpeed(MOTOR_RIGHT, speed) ;
  moveSpeed = speed; // save the value
} 

//*********************************************
//Functions to speed up or slow down the robot
//*********************************************
void moveSlower(int decrement)
{
   Serial.print(" Slower: "); 
   if( moveSpeed >= speedIncrement + MIN_SPEED)
     moveSpeed -= speedIncrement;     
   else moveSpeed = MIN_SPEED;
   moveSetSpeed(moveSpeed); 
   Serial.println(moveSpeed);
}

void moveFaster(int increment)
{
  Serial.print(" Faster: ");
  moveSpeed += speedIncrement; 
  if(moveSpeed > 100)
     moveSpeed = 100;
  moveSetSpeed(moveSpeed); 
  Serial.println(moveSpeed);  
}

int moveGetState()
{
 return moveState;  
}

// this is the low level movement state.
// it will differ from the command state when the robot is avoiding obstacles
void changeMoveState(int newState)
{
  if(newState != moveState)
  {
    Serial.print("Changing move state from "); Serial.print( states[moveState]);
    Serial.print(" to "); Serial.println(states[newState]);
    moveState = newState;
  } 
} 

//*****************************
//Functions to rotate the robot
//*****************************
void moveRotate(int angle)
{
  Serial.print("Rotating ");  Serial.println(angle);
  if(angle < 0)
  {
    Serial.println(" (left)"); 
    motorReverse(MOTOR_LEFT,  moveSpeed); 
    motorForward(MOTOR_RIGHT, moveSpeed);  
    angle = -angle;
	changeMoveState(MOV_ROTATE);
  }
  else if(angle > 0)
  {
    Serial.println(" (right)");
    motorForward(MOTOR_LEFT,  moveSpeed);
    motorReverse(MOTOR_RIGHT, moveSpeed);
    changeMoveState(MOV_ROTATE);
  }  
  int ms = rotationAngleToTime(angle, moveSpeed);
  movingDelay(ms); 
  //moveBrake();   
  moveStop();
}

void moveStartRotation(int dir)
{
  Serial.print("Rotating dir:");  Serial.println(dir);
  if(dir == DIR_LEFT)
  {
    Serial.println(" (left)"); 
    motorReverse(MOTOR_LEFT,  moveSpeed); 
    motorForward(MOTOR_RIGHT, moveSpeed);  
    changeMoveState(MOV_ROTATE);
  }
  else if(dir == DIR_RIGHT)
  {
    Serial.println(" (right)");
    motorForward(MOTOR_LEFT,  moveSpeed);
    motorReverse(MOTOR_RIGHT, moveSpeed);
    changeMoveState(MOV_ROTATE);
  }  
}

// return the time in milliseconds to turn the given angle at the given speed
long rotationAngleToTime( int angle, int speed)
{
int fullRotationTime; // time to rotate 360 degrees at given speed

  if(speed < MIN_SPEED)
    return 0; // ignore speeds slower then the first table entry

  angle = abs(angle);

  if(speed >= 100)
    fullRotationTime = rotationTime[NBR_SPEEDS-1]*Drag; // the last entry is 100%
  else
 { 
    int index = (speed - MIN_SPEED) / SPEED_TABLE_INTERVAL; // index into speed and time tables
    int t0 =  rotationTime[index]*Drag;
    int t1 = rotationTime[index+1]*Drag;    // time of the next higher speed 
    fullRotationTime = map(speed,  speedTable[index],  speedTable[index+1], t0, t1);  
   // Serial.print("index= ");  Serial.print(index); Serial.print(", t0 = "); Serial.print(t0);
   // Serial.print(", t1 = ");  Serial.print(t1); 
 }
 // Serial.print(" full rotation time = ");  Serial.println(fullRotationTime);
  long result = map(angle, 0,360, 0, fullRotationTime);
  return result; 
}



// rotate the robot from MIN_SPEED to 100% increasing by SPEED_TABLE_INTERVAL
/*void calibrateRotationRate(int direction, int angle)
{  
  Serial.print(locationString[direction]);
  Serial.println(" calibration" );    
  for(int speed = MIN_SPEED; speed <= 100; speed += SPEED_TABLE_INTERVAL)
  { 
    delay(1000);
    //blinkNumber(speed/10);   
 
    if( direction == DIR_LEFT)
    {    // rotate left
      motorReverse(MOTOR_LEFT,  speed); 
      motorForward(MOTOR_RIGHT, speed);  
    }
    else if( direction == DIR_RIGHT)
    {    // rotate right
      motorForward(MOTOR_LEFT,  speed);
      motorReverse(MOTOR_RIGHT, speed);
    }
    else
       Serial.println("Invalid direction");
    

    int time = rotationAngleToTime(angle, speed);

    Serial.print(locationString[direction]);
    Serial.print(": rotate ");           Serial.print(angle);
    Serial.print(" degrees at speed ");  Serial.print(speed);
    Serial.print(" for ");               Serial.print(time);
    Serial.println("ms");           
    delay(time); 
    motorStop(MOTOR_LEFT);
    motorStop(MOTOR_RIGHT);  
    delay(2000); // two second delay between speeds
  }    
}*/

/************* high level movement functions ****************/

//moves in the given direction at the current speed for the given duration in milliseconds
void timedMove(int direction, int duration)
{
  Serial.print("Timed move ");
  if(direction == MOV_FORWARD) {
    Serial.println("forward");
    moveForward();    
  }
  else if(direction == MOV_BACK) {
    Serial.println("back");
    moveBackward();     
  }
  else
    Serial.println("?");
    
  movingDelay(duration);
  moveStop();
}

// check for obstacles while delaying the given duration in ms
void movingDelay(long duration)
{
  long startTime = millis();
  while(millis() - startTime < duration) {
     // function in Look module checks for obstacle in direction of movement 
     checkMovement();
  }  
} 
