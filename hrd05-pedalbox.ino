/*
      __  __                          ____             _                ____  _       _      _           
     / / / /___ _____  ____  ___     / __ \____ ______(_)___  ____ _   / __ \(_)   __(_)____(_)___  ____ 
    / /_/ / __ `/ __ \/_  / / _ \   / /_/ / __ `/ ___/ / __ \/ __ `/  / / / / / | / / / ___/ / __ \/ __ \
   / __  / /_/ / / / / / /_/  __/  / _, _/ /_/ / /__/ / / / / /_/ /  / /_/ / /| |/ / (__  ) / /_/ / / / /
  /_/ /_/\__,_/_/ /_/ /___/\___/  /_/ |_|\__,_/\___/_/_/ /_/\__, /  /_____/_/ |___/_/____/_/\____/_/ /_/ 
  
  Pedalbox code for the HRD05
  Mark Oosting, 2019
  
*/

#include <CAN.h> // https://github.com/sandeepmistry/arduino-CAN

#define TPS1_IN 0
#define TPS2_IN 1
#define BPS1_IN 2
#define BPS2_IN 3

#define MAX_DIFF 0.075 // 7.5% maximum difference
#define POT_RATIO 0.74  // Electrical ratio between POTs

int THROTTLE = 0;
int BRAKE    = 0;

int TPS1 = 0;
int TPS2 = 0;

void setup() {
  Serial.begin(115200);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

void loop() {
  
  THROTTLE = plausibility_check(analogRead(TPS1_IN), analogRead(TPS2_IN), 0.74);
  BRAKE = plausibility_check(analogRead(BPS1_IN), analogRead(BPS2_IN), 0.74);

//  Serial.print(analogRead(TPS1_IN));
//  Serial.print("\t");
//  Serial.print(analogRead(TPS2_IN));
//  Serial.println();

  THROTTLE = map(THROTTLE, 784, 349, 0, 255);
  THROTTLE = constrain(THROTTLE, 0, 255); 

  BRAKE = map(BRAKE, 784, 349, 0, 255);
  BRAKE = constrain(BRAKE, 0, 255);

  // Prevent BSPD from triggering
  // - 5kW from 66kW is 7.5%, so 19 of 255.
  // - 50% of brake is considered 'hard braking' for now.
  if(THROTTLE >= 19 && BRAKE >= 127) {
    THROTTLE = 0;
    BRAKE = 0;
  }
  
  Serial.println(THROTTLE);
  Serial.println(BRAKE);
  
  CAN.beginPacket(0x12);
  CAN.write(THROTTLE);
  CAN.write(BRAKE);
  CAN.endPacket();
  
  delay(64);
}

int plausibility_check(int POT1, int POT2, float POT_RATIO){

  int OUT = 0;
  
  float RATIO = (float)POT1 / (float)POT2;
  float DIFF = abs(RATIO - POT_RATIO);

  if(DIFF >= MAX_DIFF){
    Serial.println("IMPLAUSIBLE SIGNAL");
    THROTTLE = 0;
    BRAKE = 0;
  } else {
    OUT = (POT1 + POT2) / 2;
  }

  return OUT;
}
