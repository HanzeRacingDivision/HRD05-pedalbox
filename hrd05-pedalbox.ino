/*
      __  __                          ____             _                ____  _       _      _           
     / / / /___ _____  ____  ___     / __ \____ ______(_)___  ____ _   / __ \(_)   __(_)____(_)___  ____ 
    / /_/ / __ `/ __ \/_  / / _ \   / /_/ / __ `/ ___/ / __ \/ __ `/  / / / / / | / / / ___/ / __ \/ __ \
   / __  / /_/ / / / / / /_/  __/  / _, _/ /_/ / /__/ / / / / /_/ /  / /_/ / /| |/ / (__  ) / /_/ / / / /
  /_/ /_/\__,_/_/ /_/ /___/\___/  /_/ |_|\__,_/\___/_/_/ /_/\__, /  /_____/_/ |___/_/____/_/\____/_/ /_/ 
  
  Pedalbox code for the HRD05
  Mark Oosting, 2019
  
*/

#include <SPI.h>
#include <mcp_can.h>

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

void setup() {
  Serial.begin(9600);
}

void loop() {
  
  THROTTLE = plausibility_check(analogRead(TPS1_IN), analogRead(TPS2_IN));
  // BRAKE = plausibility_check(analogRead(BPS1_IN), analogRead(BPS2_IN));
  
  Serial.println(THROTTLE);
  
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
