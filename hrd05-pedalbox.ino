#include <SPI.h>
#include "variables.h"
#include "can.h"

void setup() {

  Serial.begin(9600);
  SPI.begin();             ///Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
  
  last_read = millis();
  
  pinMode(BPS,    INPUT);
  pinMode(buzzer, OUTPUT);
                        
  can_setup();

  delay(1000);
  
  awaitRTD();

}

void loop() {

    // Get up-to-date with the CAN bus
    if (mcp2515.readMessage(&MSG) == MCP2515::ERROR_OK)
    {
      if(DMC_TRQS.can_id == MSG.can_id) {
        motor_rpm = MSG.data[6];
        Serial.println(motor_rpm); 
      }
    }


    /// Actual pedal box logic ////////////////////////////////////////////////////////////////////////

    throttle = 0;
    
    if(millis() - last_read >= DATA_RATE) {
      APPS1 = analogRead(APPS1_IN);
      APPS2 = analogRead(APPS2_IN);
      last_read = millis();
    }
    
    if(plausibility_check2(APPS1, APPS2) == true){
      /// Calcalute the average of both APPS values
      APPS = (APPS1+APPS2)/2; 
      
      throttle = map(APPS, 0, 1023, 0, 33);  /// Torque request: map the throttle value from 0 to 33 Nm
      throttle = constrain(throttle, 0, 33); /// Torque request: never exceed 33 Nm  
    }

    /// Prevent BSPD from triggering (torque request > 5Nm and braking more than 50%)
    if(throttle > 5 && brake > 50){
      throttle = 0;
    }

    /// TODO: if speed < 5kmh and throttle is 0, EnableRq should be 0
 
    if(millis() - last_can >= DATA_RATE){
    
//      Serial.print("Throttle setting is: ");
//      Serial.println(throttle);

      send_DMC_CTRL(throttle);
      
    }

    /* tractive system is off logic
    if(){
      awaitRTD();  
    }
    */

}

/// Logic for the Ready To Drive mode. 
void awaitRTD() {
  
  send_DMC_standby();
  
  while (1)
  {
    if (mcp2515.readMessage(&MSG) == MCP2515::ERROR_OK)
    {
      
      Serial.println(); 
       
      int readyToDrive = MSG.data[0];  /// Read RTD button from the Dashboard
      int tsReady      = MSG.data[1];  /// Read TS status from the Dashboard

      if(MSG.can_id == DASH_MSG.can_id) {
        Serial.print("Dashboard RTD: "); 
        Serial.println(readyToDrive); 
        Serial.print("Dash TS status: "); 
        Serial.println(tsReady); 
      }
      
      if(DMC_TRQS.can_id == MSG.can_id) {
        DMC_Ready = MSG.data[0];
        Serial.print("DMC_Ready: ");
        Serial.print(DMC_Ready);
      }

       
      // && digitalRead(BPS) == 1)
      if(DMC_Ready    == 1 && 
         readyToDrive == 1 && 
         tsReady      == 1) {
          tone(buzzer, 800, 3000);
          break;
      }
    }
  }
}


bool plausibility_check(float POT1, float POT2) {   ///Plausibility with ratio by dividing. This will depend on how we place the sensors

  bool PLAUSIBLE = false;

  float RATIO = POT1 / POT2;                        ///Ratio between the two potentiometers
  float DIFF = abs(RATIO - POT_RATIO);              ///Calculating the absolute difference between them, keeping in mind the defined ratio

  if (DIFF > MAX_DIFF) {
    PLAUSIBLE = false;
  } else {
    PLAUSIBLE = true;
  }
  
  return PLAUSIBLE;
}

bool plausibility_check2(float POT1, float POT2) {  ///Plausibility with ratio by subtraction. This is used when the sensors have an offset relative to each other.

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
