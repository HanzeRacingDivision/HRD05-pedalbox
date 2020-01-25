/*
      __  __                          ____             _                ____  _       _      _
     / / / /___ _____  ____  ___     / __ \____ ______(_)___  ____ _   / __ \(_)   __(_)____(_)___  ____
    / /_/ / __ `/ __ \/_  / / _ \   / /_/ / __ `/ ___/ / __ \/ __ `/  / / / / / | / / / ___/ / __ \/ __ \
   / __  / /_/ / / / / / /_/  __/  / _, _/ /_/ / /__/ / / / / /_/ /  / /_/ / /| |/ / (__  ) / /_/ / / / /
  /_/ /_/\__,_/_/ /_/ /___/\___/  /_/ |_|\__,_/\___/_/_/ /_/\__, /  /_____/_/ |___/_/____/_/\____/_/ /_/

  Pedalbox code for the HRD05
  Mark Oosting, 2020
  Armand Micu, 2020

*/

#include <SPI.h>
#include "variables.h"
#include "variables_dmc.h"
#include "can.h"

void setup() {

  Serial.begin(9600);
  SPI.begin();                     /// Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
  
  pinMode(BPS, INPUT);
  pinMode(BUZZER, OUTPUT);

  last_read = millis();
  
  CAN_setup();
  
  delay(1000);                    /// Wait on the inverter to get ready
  
  awaitRTD();

}

void loop() {

    CAN_update();

    THROTTLE = 0;
    
    if(millis() - last_read >= DATA_RATE) {
      APPS1 = analogRead(APPS1_IN);
      APPS2 = analogRead(APPS2_IN);
      last_read = millis();
    }

    if(is_plausible(APPS1, APPS2)){
      
      APPS = (APPS1+APPS2)/2;                               /// Calcalute the average of both APPS values
      
      THROTTLE = map(APPS, 0, 1023, 0, 33);                 /// Torque request: map the throttle value from 0 to 33 Nm
      THROTTLE = constrain(THROTTLE, 0, 33);                /// Torque request: never exceed 33 Nm
      
    }
    
    if(THROTTLE > 5 && BRAKE > 50){                         /// Prevent BSPD from triggering (torque request > 5Nm and braking more than 50%)
      THROTTLE = 0;
    }
 
    if(millis() - last_can >= DATA_RATE){
    
       if ( THROTTLE == 0 && DMC_SpdAct < min_rpm ){         /// Prevent bucking by disabling control under a minium speed. DMC_SpdAct is the actual RPM of the motor.
          DMC_EnableRq = 0;
          THROTTLE = 0;
       } else { 
          DMC_EnableRq = 1;
       }
       
       send_DMC_CTRL(THROTTLE, DMC_EnableRq);  
       
    }

    /* tractive system is off logic
    if(){
      awaitRTD();  
    }
    */

}


void awaitRTD() {                                             /// Logic for the Ready To Drive mode. 
  
  send_DMC_standby();
  
  while (1)
  {
    if (mcp2515.readMessage(&MSG) == MCP2515::ERROR_OK)
    {

      if(MSG.can_id == DASH_MSG.can_id) {
       
        ReadyToDrive = MSG.data[0];                           /// Read RTD button from the Dashboard
        TSReady      = MSG.data[1];                           /// Read TS status from the Dashboard
      
        Serial.print("Dashboard RTD: "); 
        Serial.println(ReadyToDrive); 
        Serial.print("Dash TS status: "); 
        Serial.println(TSReady); 
      }

      if(DMC_TRQS.can_id == MSG.can_id) {
        DMC_Ready = MSG.data[0];
        
        Serial.print("DMC_Ready: ");
        Serial.print(DMC_Ready);
      }
      
      Serial.println();

      // && digitalRead(BPS) == 1)
      if(DMC_Ready    == 1 && 
         ReadyToDrive == 1 && 
         TSReady      == 1) {
          tone(BUZZER, 800, 3000);
          break;
      }
    }
  }
}


bool is_plausible(float POT1, float POT2) {             /// Plausibility with ratio by dividing. This will depend on how we place the sensors

  bool PLAUSIBLE = false;

  float RATIO = POT1 / POT2;                            /// Ratio between the two sensors
  float DIFF = abs(RATIO - APPS_RATIO);                 /// Calculating the absolute difference between them

  if (DIFF > MAX_DIFF) {                                /// Compare the sensor difference to the defined maximum error
    PLAUSIBLE = false;
  } else {
    PLAUSIBLE = true;
  }
  
  return PLAUSIBLE;
}

bool is_plausible2(float POT1, float POT2) {            /// Plausibility with ratio by subtraction. This is used when the sensors have an offset relative to each other.

  bool PLAUSIBLE = false;
  
  float RATIO = abs(POT1-POT2);
  float DIFF = abs(RATIO-POT_OFFSET);
  
  if (DIFF > MAX_DIFF) {
    PLAUSIBLE = false;
  } else {
    PLAUSIBLE = true;
  }

  return PLAUSIBLE;
}
