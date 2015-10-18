/** \file IRControl.ino
 * code for ping distance sensor.
 * A more elaborated file description.
 */
 
 #ifdef ENABLE_IR
 /**
 * A global integer value.
 * More details about this value.
 */
void process_IR(void)
{
  int right_run, left_run;
  right_run = getRunState(MOTOR_RIGHT);
  left_run = getRunState(MOTOR_LEFT);
  
  command = CMD_NONE;
  decode_results results;
  if (irrecv.decode(&results)) 
  {
    if (results.value == KEY_2 )
    {
      command = CMD_FW;
      iKey = KEY_2;
    }
    else if (results.value == KEY_4)
    {
      if (left_run == FORWARD || right_run == FORWARD) command = CMD_FW_LEFT;
      else if (left_run == BACKWARD || right_run == BACKWARD) command = CMD_BW_LEFT;
      else 
      {
        command = CMD_FW_LEFT;
        b_no_move = true;
      }
      iKey = KEY_4;
    }
    else if (results.value == KEY_5)
    {
      command = CMD_STOP;
      iKey = KEY_5;
    }
    else if (results.value == KEY_6)
    {
      if (left_run == FORWARD || right_run == FORWARD) command = CMD_FW_RIGHT;
      else if (left_run == BACKWARD || right_run == BACKWARD) command = CMD_BW_RIGHT;
      else 
      {
        command = CMD_FW_RIGHT;
        b_no_move = true;
      }
      iKey = KEY_6;
    } 
    else if (results.value == KEY_MENOS)
    {
      command = CMD_DECREASE;
      iKey = KEY_MENOS;
    }  
    else if (results.value == KEY_MAS)
    {
      command = CMD_INCREASE;
      iKey = KEY_MAS;
    } 
    else if (results.value == KEY_8)
    {
      command = CMD_BW;
      iKey = KEY_8;
    } 
   else if (results.value == 4294967295)
    {
      timer =7000; 
      if ((iKey != KEY_MENOS) & (iKey != KEY_MAS)) 
      {
        b_pulsacion_larga = true;
        //Serial.println("pulsacion larga\n");
      }
    } 
     else
    {
      command = CMD_NONE;
      //Serial.println(results.value, HEX);
      //Serial.println(results.value);
    } 
    irrecv.resume(); // Receive the next value
  }
  else  // no code received
  {
    if (b_pulsacion_larga == true)  //Si hay pulsacion larga
    {
      if (timer > 0) --timer;      // aun se recibe pulsacion larga
      else                        // ya no rebimos pulsacion larga
      {
        b_pulsacion_larga = false;
        //Serial.println(left_run,right_run);
        if (b_no_move == true){b_no_move=false;command = CMD_STOP;}
        else if (left_run == FORWARD && right_run == FORWARD) command = CMD_STOP;
        else if (left_run == BACKWARD && right_run == BACKWARD) command = CMD_STOP;
        else if (left_run == FORWARD || right_run == FORWARD) command = CMD_FW;
        else if (left_run == BACKWARD || right_run == BACKWARD) command = CMD_BW;
        else
          command = CMD_NONE;
      }
    }
  }
}

#endif