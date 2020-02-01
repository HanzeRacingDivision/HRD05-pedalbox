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
#include "variables.h"              /// Variables specific to the pedalbox
#include "variables_dmc.h"          /// Variables used for the DMC514 inverter
#include "pin_mapping.h"            /// Arduino pin mapping + input and output settings
#include "can.h"                    /// Our own CAN variables and code

unsigned long last_read;            /// Millis when last loop was executed.

void setup() {

  Serial.begin(9600);
  SPI.begin();                      /// Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.

  last_read = millis();

  PIN_setup();
  CAN_setup();
  
  delay(1000);                      /// Wait on the inverter to get ready
  
  awaitRTD();

}

void loop() {
  
    CAN_update();

    if(!car_is_ready()){
        awaitRTD();
    }
    
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
    
    if ( THROTTLE == 0 && DMC_SpdAct < min_speed ){         /// Prevent bucking by disabling control under a minium speed.
      DMC_EnableRq = 0;
      DMC_TrqRq = 0;
      THROTTLE = 0;
    } else {
      DMC_EnableRq = 1;
      DMC_TrqRq = THROTTLE;
    }
    
    if(millis() - last_can >= DATA_RATE){                   /// Send CAN to the inverter. 
    
      Serial.print("SENDING APPS");
      send_DMC_CTRL(33, DMC_EnableRq);
       
    }

}


void awaitRTD() {                                             /// Logic for the Ready To Drive mode. 
  
  DMC_Ready = 0;
  TSReady = 0;
  ReadyToDrive = 0;
  
  send_DMC_standby(0);
  
  while (1) {

    Serial.print("Awaiting ready state:");
    Serial.println();
    Serial.print(DMC_Ready);
    Serial.print(ReadyToDrive);
    Serial.print(TSReady);
    Serial.println();
    Serial.println();

    CAN_update();

    if(car_is_ready()) {
        tone(BUZZER, 800, 3000);
        break;
    }

    delay(DATA_RATE);
  }
}

bool car_is_ready() {
  return (DMC_Ready && ReadyToDrive && TSReady);        /// Needs the brake pedal status as well. 
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
