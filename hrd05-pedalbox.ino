/*
      __  __                          ____             _                ____  _       _      _
     / / / /___ _____  ____  ___     / __ \____ ______(_)___  ____ _   / __ \(_)   __(_)____(_)___  ____
    / /_/ / __ `/ __ \/_  / / _ \   / /_/ / __ `/ ___/ / __ \/ __ `/  / / / / / | / / / ___/ / __ \/ __ \
   / __  / /_/ / / / / / /_/  __/  / _, _/ /_/ / /__/ / / / / /_/ /  / /_/ / /| |/ / (__  ) / /_/ / / / /
  /_/ /_/\__,_/_/ /_/ /___/\___/  /_/ |_|\__,_/\___/_/_/ /_/\__, /  /_____/_/ |___/_/____/_/\____/_/ /_/

  Pedalbox code for the HRD05
  Mark Oosting, 2019
  Joram de Poel, 2019
  Vincent van der Woude, 2019

*/

#include <CAN.h> // https://github.com/sandeepmistry/arduino-CAN

#define TPS1_IN 0
#define TPS2_IN 1
#define BPS1_IN 2
#define BPS2_IN 3

int MAX_DIFF_MS = 100;

int THROTTLE = 0;
int BRAKE    = 0;

int TPS1_OFFSET = 200;       //hardware offset for pot 2 to create an offset
int BPS1_OFFSET = 200;

unsigned long last = 0;     //millis when last loop was executed
int looptime = 20;          //execute the loop every 20 ms and read sensors continiously

unsigned long firstTPSErrorTime = 0;      //millis when the values were different for the first time
unsigned long firstBPSErrorTime = 0;

bool first_TPS_Error = false;     //is the first TPS error time recorded? this is used for measuring the 100 ms which are allowed to have a different sensor value
bool first_BPS_Error = false;

bool TPS_Error = true;          //do a reading first to clear the error.
bool BPS_Error = true;

int LPF_TPS1[50];           //low pass filter TPS sensor 1
int LPF_TPS2[50];
int LPF_BPS1[50];
int LPF_BPS2[50];

int AV_TPS1 = 0;          //average value after filter for TPS 1
int AV_TPS2 = 0;
int AV_BPS1 = 0;
int AV_BPS2 = 0;

void setup() {
  Serial.begin(115200);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

void loop() {

  //continuesly run this piece of code
  do {
    LPF_TPS1[0] = analogRead(TPS1_IN);      //read all current sensor values
    LPF_TPS2[0] = analogRead(TPS2_IN);
    LPF_BPS1[0] = analogRead(BPS1_IN);
    LPF_BPS1[0] = analogRead(BPS2_IN);

    int AV_TPS1 = 0;                        //average to 0
    int AV_TPS2 = 0;
    int AV_BPS1 = 0;
    int AV_BPS2 = 0;
    for (int i = 49; i > 0; i--) {
      LPF_TPS1[i] = LPF_TPS1[i - 1];        //shift all values by one
      LPF_TPS2[i] = LPF_TPS2[i - 1];
      LPF_BPS1[i] = LPF_BPS1[i - 1];
      LPF_BPS2[i] = LPF_BPS2[i - 1];
      AV_TPS1 += LPF_TPS1[i];               //add value for average
      AV_TPS2 += LPF_TPS2[i];
      AV_BPS1 += LPF_BPS1[i];
      AV_BPS2 += LPF_BPS2[i];
    }
    AV_TPS1 /= 50;
    AV_TPS2 /= 50;
    AV_BPS1 /= 50;
    AV_BPS2 /= 50;

  } while (millis() - last > looptime);

  THROTTLE = plausibility_check(AV_TPS1, AV_TPS2, true);
  BRAKE = plausibility_check(AV_BPS1, AV_BPS2, false);


  THROTTLE = map(THROTTLE, 784, 349, 0, 255);
  THROTTLE = constrain(THROTTLE, 0, 255);

  BRAKE = map(BRAKE, 784, 349, 0, 255);
  BRAKE = constrain(BRAKE, 0, 255);

  // Prevent BSPD from triggering
  // - 5kW from 66kW is 7.5%, so 19 of 255.
  // - 50% of brake is considered 'hard braking' for now.
  if (THROTTLE >= 19 && BRAKE >= 127) {
    THROTTLE = 0;
    BRAKE = 0;        //this means you release the brake??? whouldn't you keep sending the correct value?
  }

  Serial.println(THROTTLE);
  Serial.println(BRAKE);

  CAN.beginPacket(0x12);
  CAN.write(THROTTLE);
  CAN.write(BRAKE);
  CAN.write(TPS_Error);
  CAN.write(BPS_Error);
  CAN.endPacket();

  last = millis();        //reset loop timer
}



int plausibility_check(int POT1, int POT2, bool isTPS) {

  if ((POT1 + (POT1 / 10)) < POT2 || (POT2 + (POT2 / 10)) < POT1) { //if on pot is more than 10% lager or smaller than the other    !! PLEASE CHECK THIS PART !!
    return 0;
    if (isTPS) {
      if (first_TPS_Error) {              //if it is the TPS sensor and the error flag is not yet set
        firstTPSErrorTime = millis();         //set current time for the first error
      }
      if (millis() - firstTPSErrorTime > MAX_DIFF_MS) { //AAAAAAAAAAAAAAHHHH SOMETHING WENT HORRBLY WRONG< STOP THE CAR!!!
        TPS_Error = true;
        THROTTLE = 0;
        BRAKE = 0;
      }
    }
    else {
      if (first_BPS_Error) {              //if it is the BPS sensor and the error flag is not yet set
        firstBPSErrorTime = millis();         //set current time for the first error
      }
      if (millis() - firstBPSErrorTime > MAX_DIFF_MS) { //AAAAAAAAAAAAAAHHHH SOMETHING WENT HORRBLY WRONG< STOP THE CAR!!!
        BPS_Error = true;
        THROTTLE = 0;
        BRAKE = 0;
      }
    }
  }

  else {    //everything is doing fine
    return (POT1 + POT2) / 2;     //return average from both POTS

    if (isTPS) {
      first_TPS_Error = true;       //reset flag
      TPS_Error = false;
    } else {
      first_BPS_Error = true;       //reset flag
      BPS_Error = false;
    }
  }
}

/*      Marks shit :)
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
*/
