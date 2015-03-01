/*********************************
 code for ping distance sensor
**********************************/

// Returns the distance in cm
// this returns 0 if no ping sensor is connected or the distance is greater than around 4m
int getDistance(int Sonar)
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, cm, suma=0;

  for (int x=0; x < 3; x++)
  {
    // The PING))) is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    if (Sonar == SONAR1)
    {
      digitalWrite(SONAR1_TRIG_PIN, LOW);
      delayMicroseconds(5);
      digitalWrite(SONAR1_TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(SONAR1_TRIG_PIN, LOW);
    
      duration = pulseIn(SONAR1_ECHO_PIN, HIGH/*, 20000*/); // if a pulse does not arrive 
                                                // in 20 ms then the ping sensor
    }  
    else if (Sonar == SONAR2)
    {
      digitalWrite(SONAR2_TRIG_PIN, LOW);
      delayMicroseconds(5);
      digitalWrite(SONAR2_TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(SONAR2_TRIG_PIN, LOW);
    
      duration = pulseIn(SONAR2_ECHO_PIN, HIGH, 8000); // if a pulse does not arrive 
                                                // in 20 ms then the ping sensor
    }
    
    // is not connected
    if(duration >=16000)
      return 0;
    
    suma = suma + duration;
  }  
  duration = suma / 3;
  // convert the time into a distance
  //cm = int(0.017*duration);  // for 5V
  cm = int(duration/40);  // correcion debido a que elvoltaje es 4.8 voltios en lugar de 5V
  
  Serial.println(cm);
  return cm ; // convert cm
}


// -------------------------------
// -------------------------------
void checkMovement()
{
  int distance = 0;
  if (moveGetState() == MOV_BACK)
  {
    distance = getDistance(SONAR2);
  }
  else if (moveGetState() == MOV_FORWARD)
  {
    distance = getDistance(SONAR1);
  }
  if((distance <= MIN_DISTANCE) && (distance != 0)) 
  {
    moveStop();
    roam();
    command = CMD_STOP;
  }
}

int lookAt(int dir)
{
  switch(dir)
  {
    case(DIR_CENTER):
      myservo.write(MID_MOVE_SONAR);
    break;
    case(DIR_LEFT):
      myservo.write(MAX_MOVE_SONAR);
    break;
    case(DIR_RIGHT):
      myservo.write(MIN_MOVE_SONAR);
    break;
  }
  delay(500);  //Espera el el servo llegue a la posicion
  return getDistance(SONAR1);
}

// Look for and avoid obstacles by rotating robot  
void roam()
{
  //int distance;
  Serial.println("Scanning:"); 
  int leftDistance  = lookAt(DIR_LEFT);     
  if(leftDistance > CLEAR_DISTANCE)  {
    Serial.print(" rotation left: ");
   lookAt(DIR_CENTER);
    moveStartRotation(DIR_LEFT);
    do //rotar
    {
      leftDistance = getDistance(SONAR1);
    }while(leftDistance < CLEAR_DISTANCE);
    moveStop();
    // avanzar para superar el obstaculo
  }
  else {
    delay(500);
    int rightDistance = lookAt(DIR_RIGHT);
    if(rightDistance > CLEAR_DISTANCE) {
    //  Serial.println(" moving right: ");
      lookAt(DIR_CENTER);
      moveStartRotation(DIR_RIGHT);
      do //rotar
      {
        rightDistance = getDistance(SONAR1);
      }while(rightDistance < CLEAR_DISTANCE);
      moveStop();  
    }
    else {
     // Serial.print(" no clearence : ");
      lookAt(DIR_CENTER);
      int distance = max( leftDistance, rightDistance);
      if(distance < CLEAR_DISTANCE/2) {
        timedMove(MOV_BACK, 1000); // back up for one second  
        moveRotate(-180); // turn around
      }
      else {
        if(leftDistance > rightDistance)
          moveRotate(-90);
        else
          moveRotate(90);   
      }                  
    } 
  } 
}
