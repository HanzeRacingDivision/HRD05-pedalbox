/*
      __  __                          ____             _                ____  _       _      _           
     / / / /___ _____  ____  ___     / __ \____ ______(_)___  ____ _   / __ \(_)   __(_)____(_)___  ____ 
    / /_/ / __ `/ __ \/_  / / _ \   / /_/ / __ `/ ___/ / __ \/ __ `/  / / / / / | / / / ___/ / __ \/ __ \
   / __  / /_/ / / / / / /_/  __/  / _, _/ /_/ / /__/ / / / / /_/ /  / /_/ / /| |/ / (__  ) / /_/ / / / /
  /_/ /_/\__,_/_/ /_/ /___/\___/  /_/ |_|\__,_/\___/_/_/ /_/\__, /  /_____/_/ |___/_/____/_/\____/_/ /_/ 
  
  Pedalbox code for the HRD05
  Mark Oosting, 2019
  
*/

//#include <SPI.h>
//#include <mcp_can.h>
#include <CAN.h>          // https://github.com/sandeepmistry/arduino-CAN

#define TPS1_IN 0
#define TPS2_IN 1
#define BPS1_IN 2
#define BPS2_IN 3

#define MAX_DIFF 0.075 // 7.5% maximum difference
#define POT_RATIO 0.3  // Electrical ratio between POTs

int THROTTLE = 0;
int BRAKE    = 0;

int TPS1 = 0;
int TPS2 = 0;

// MCP_CAN CAN(10);

void setup() {
  Serial.begin(115200);
  
//  while (CAN_OK != CAN.begin(CAN_500KBPS)){
//      Serial.println("CAN BUS init Failed");
//      delay(100);
//  }
//  Serial.println("CAN BUS Shield Init OK!");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

void loop() {
  
  THROTTLE = plausibility_check(analogRead(TPS1_IN), analogRead(TPS2_IN));
  // BRAKE = plausibility_check(analogRead(BPS1_IN), analogRead(BPS2_IN));
  
  THROTTLE = map(THROTTLE, 657, 254, 0, 255);
  THROTTLE = constrain(THROTTLE, 0, 255);
  Serial.println(THROTTLE);
  
  CAN.beginPacket(0x12);
  CAN.write(THROTTLE);
  CAN.write(BRAKE);
  CAN.endPacket();
  
  delay(50);
}

int plausibility_check(int POT1, int POT2){

  int OUT = 0;
  
  float RATIO = (float)POT1 / (float)POT2;
  float DIFF = abs(RATIO - POT_RATIO);

  if(DIFF >= MAX_DIFF){
    Serial.println("IMPLAUSIBLE SIGNAL");
  } else {
    OUT = (POT1 + POT2) / 2;
  }

  return OUT;
}
