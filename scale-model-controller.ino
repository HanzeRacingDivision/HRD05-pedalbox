/*
      __  __                          ____             _                ____  _       _      _
     / / / /___ _____  ____  ___     / __ \____ ______(_)___  ____ _   / __ \(_)   __(_)____(_)___  ____
    / /_/ / __ `/ __ \/_  / / _ \   / /_/ / __ `/ ___/ / __ \/ __ `/  / / / / / | / / / ___/ / __ \/ __ \
   / __  / /_/ / / / / / /_/  __/  / _, _/ /_/ / /__/ / / / / /_/ /  / /_/ / /| |/ / (__  ) / /_/ / / / /
  /_/ /_/\__,_/_/ /_/ /___/\___/  /_/ |_|\__,_/\___/_/_/ /_/\__, /  /_____/_/ |___/_/____/_/\____/_/ /_/

  Pedalbox & stuur code for the HRD05
  Mark Oosting, Jeroen Langebeeke, 2019
*/



#include <CAN.h> // https://github.com/sandeepmistry/arduino-CAN
//Microchip MCP2515 wiring:
//Microchip MCP2515:  Arduino:
//  VCC                   5V
//  GND                   GND
//  SCK                   SCK (digitaal pin 13)
//  SO                    MISO (digitaal pin 12)
//  SI                    MOSI (digitaal pin 11)
//  CS                    10
//  INT                   2

#define TPS1_IN 0
#define TPS2_IN 1
#define BPS1_IN 2
#define BPS2_IN 3
#define STEER_IN 4          // potmeter op analoog poort 4

#define MAX_DIFF 0.075 // 7.5% maximum difference
#define POT_RATIO 0.74  // Electrical ratio between POTs

#define maxAfwijking 100      // de maximale waarde die de potmeter mag verdraaien per 1/50 seconde    

int THROTTLE = 0;
int BRAKE    = 0;
int STEER    = 0;
int POT      = 0;

int TPS1 = 0;
int TPS2 = 0;


void setup() {
  Serial.begin(115200);
  // start the CAN bus at 500 kbps
  if (!CAN.begin(250E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}


void loop() {

  THROTTLE = plausibility_check(analogRead(TPS1_IN), analogRead(TPS2_IN));
  BRAKE = plausibility_check(analogRead(BPS1_IN), analogRead(BPS2_IN));

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

  if (THROTTLE >= 19 && BRAKE >= 127) {
    THROTTLE = 0;
    BRAKE = 0;
  }
  // de stuurhoek sensor simpel uitlezen.
  POT = potmeterCheck(POT);           
  STEER = map(POT, 100, 900, 0, 255);

  if (STEER <= 0) STEER = 128;
  else if (STEER >= 255) STEER = 128;
  
  STEER = constrain(STEER, 0, 255);


  Serial.print("throttle: \t ");
  Serial.print(THROTTLE);
  Serial.print("brake: \t ");
  Serial.print(BRAKE);
  Serial.print("steer: \t ");
  Serial.println(STEER);

  CAN.beginPacket(0x12);
  CAN.write(THROTTLE);
  CAN.write(BRAKE);
  CAN.write(STEER);
  CAN.endPacket();
  delay(25);

}

int plausibility_check(int POT1, int POT2) {

  int OUT = 0;
  float RATIO = (float)POT1 / (float)POT2;
  float DIFF = abs(RATIO - POT_RATIO);

  if (DIFF >= MAX_DIFF) {
    Serial.println("IMPLAUSIBLE SIGNAL");
    THROTTLE = 0;
    BRAKE = 0;
  } else {
    OUT = (POT1 + POT2) / 2;
  }
  return OUT;
}


int potmeterCheck(int potmeterWaarde) {
  int vorigePotmeterWaarde = potmeterWaarde;
  potmeterWaarde = analogRead(STEER_IN);                             // stuurhoek van potmeter uit lezen
  if ((abs(vorigePotmeterWaarde - potmeterWaarde)) > maxAfwijking) { // verschil tussen vorige potmeter en huidige potmeter, bij een groter verschil dan maxAfwijking gebeurt er iets
    if ((vorigePotmeterWaarde - potmeterWaarde) > 0) {               // kijken op de potmeter verandering negatief is
      potmeterWaarde = vorigePotmeterWaarde - maxAfwijking;          // omdat de verandering negatief is: de maximale afwijk er af halen.
    }
    else {
      potmeterWaarde = vorigePotmeterWaarde + maxAfwijking;          // ander is de potmeter verandering positief en tellen we daarom de maxAfwijking er bij op
    }
  }
  potmeterWaarde = constrain(potmeterWaarde, 10, 1010);
  return potmeterWaarde;
}
