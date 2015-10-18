// code to monitor 9V battery voltage

/******************************************************************
 * LED starts flashing when volage drops below warning level 
 * mark space ratio increses from 10% to 50% as voltage decreses from warning to critical
 * robot shuts down when battery below critical and led flashes SOS
 * 
 * LED mark space ratio changes from 10% to 90% as voltage increases to full
 *****************************************************************/

// Configuracion para LiPo 11.1v
//const int battery100   =  12600; // threshold for full battery (4 lines)
const int battery75 =  12000; // threshold for battery is 3 lines
const int battery50 =  11480; // threshold for battery is 2 lines
const int battery25 =  11000; // threshold for battery is 1 lines
const int battery0=  10000; // threshold to shut down 
const int batteryUSB=  5000; // threshold for Battery powered by USB


// indicates battery status using the given LED
void  batteryCheck(int &ri_power_level)
{  
   if (time_batery_check < 2) return;//1seg
  time_batery_check=0;
  int mv = batteryMv(BATTERY_MONITOR_PIN); // get battery level in millivolts
  //Serial.print("mv="); Serial.println(mv); 
   
   if(mv < batteryUSB){ /*Serial.println(" USB Power");*/ri_power_level=POWER_USB;}
   else if(mv < battery0)
   {
      //Serial.println(" BAT 0%");
      ri_power_level=POWER_0;
      //if (moveGetState()==MOV_STOP)
      {
         moveStop();
         StopServo();
         arp = 1;
      }
       
    }
    else if (mv < battery25)
    {  
      //show display 
      //Serial.println(" BAT 25%"); 
      ri_power_level=POWER_25;
    } 
    else if (mv < battery50) 
    {  
      //show display 
      //Serial.println(" BAT 50%");
      ri_power_level=POWER_50; 
    }      
    else if (mv < battery75) 
    {  
      //show display 
      //Serial.println(" BAT 75%");
      ri_power_level=POWER_75; 
    }   
    else{
      //Serial.println(" BAT 100%");
      ri_power_level=POWER_100;
    } 
}

// return the voltge on the given pin in millivolts
// see text for voltage divider resistor values
int  batteryMv(int pin )
{
#if defined(__AVR_ATmega32U4__) // is this a Leonardo board?
  const long INTERNAL_REFERENCE_MV = 2560; // leo reference is 2.56 volts
#else
  const long INTERNAL_REFERENCE_MV = 1100; // ATmega328 is 1.1 volts
#endif  
  const float R1 = 10.0;  // voltage divider resistors values, see text
  const float R2 = 1.0;    
  const float DIVISOR = R2/(R1+R2); 

  int value = 0;
  for(int i=0; i < 3; i++) {    
    value = value + analogRead(pin);
  }
  value  = value / 3; // get the average of 8 readings
  int mv = map(value, 0,1023, 0, INTERNAL_REFERENCE_MV / DIVISOR );

  //analogReference(DEFAULT); // set the reference back to default (Vcc)
  //analogRead(pin); // just to let the ADC settle ready for next reading
  //delay(10); // allow reference to stabalise

  return mv;
}




