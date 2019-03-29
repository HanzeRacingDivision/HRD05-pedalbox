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

#define APS1_IN 0
#define APS2_IN 1
#define BPS1_IN 2
#define BPS2_IN 3

//int MAX_DIFF_MS = 100;  // This parameter not used anymore; we always kill the car when we detect errors
float MAX_DIFF = 0.075;

int THROTTLE = 0;
int BRAKE    = 0;
// These offsets were never used in the code:
// int APS1_OFFSET = 200;       //hardware offset for pot 2 to create an offset
// int BPS1_OFFSET = 200;     
int POT_RATIO = 0.74;

unsigned long last = 0;     //millis when last loop was executed
int DATA_RATE = 20;          // Send CAN frame every 20 ms (and read sensors continuously)

unsigned long firstAPSErrorTime = 0;      //millis when the values were different for the first time
unsigned long firstBPSErrorTime = 0;

bool first_APS_Error = false;     //is the first APS error time recorded? this is used for measuring the 100 ms which are allowed to have a different sensor value
bool first_BPS_Error = false;

bool APS_Error = true;          //do a reading first to clear the error.
bool BPS_Error = true;
bool ERROR = true;

byte BUFFERSIZE = 50
int LPF_APS1[BUFFERSIZE];           //moving average APS sensor 1
int LPF_APS2[BUFFERSIZE];
int LPF_BPS1[BUFFERSIZE];
int LPF_BPS2[BUFFERSIZE];

long AV_APS1 = 0;          //average value after filter for APS 1
long AV_APS2 = 0;
long AV_BPS1 = 0;
long AV_BPS2 = 0;

void setup() {
  //Get initial values to fill arrays:
  AV_APS1 = analogRead(APS1_IN);  
  AV_APS2 = analogRead(APS2_IN);
  AV_BPS1 = analogRead(BPS1_IN);
  AV_BPS2 = analogRead(BPS2_IN);
  // Run for loop to fill array:
  for(int i=0; i<BUFFERSIZE; i++){  
    LPF_APS1[i] = AV_APS1
    LPF_APS2[i] = AV_APS2
    LPF_BPS1[i] = AV_BPS1
    LPF_BPS2[i] = AV_BPS2
  }

  Serial.begin(115200);

  // Start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
  last = millis();
}

void loop() {
  
  // Update moving average for pedal sensors:
  // Average to 0:
  AV_APS1 = 0;                        
  AV_APS2 = 0;
  AV_BPS1 = 0;
  AV_BPS2 = 0;

  for (int i = 0; i < (BUFFERSIZE-1); i++) {
    // Add value for average:
    AV_APS1 += LPF_APS1[i];             
    AV_APS2 += LPF_APS2[i];
    AV_BPS1 += LPF_BPS1[i];
    AV_BPS2 += LPF_BPS2[i];
    // Shift all values by one:
    LPF_APS1[i+1] = LPF_APS1[i];        
    LPF_APS2[i+1] = LPF_APS2[i];
    LPF_BPS1[i+1] = LPF_BPS1[i];
    LPF_BPS2[i+1] = LPF_BPS2[i];
  }
  // Read new current sensor values:
  LPF_APS1[0] = analogRead(APS1_IN);      
  LPF_APS2[0] = analogRead(APS2_IN);
  LPF_BPS1[0] = analogRead(BPS1_IN);
  LPF_BPS1[0] = analogRead(BPS2_IN);
  // Add new value to average:
  AV_APS1 += LPF_APS1[0];               
  AV_APS2 += LPF_APS2[0];
  AV_BPS1 += LPF_BPS1[0];
  AV_BPS2 += LPF_BPS2[0];
  // Divide sum to find average value:
  AV_APS1 /= BUFFERSIZE;  
  AV_APS2 /= BUFFERSIZE;
  AV_BPS1 /= BUFFERSIZE;
  AV_BPS2 /= BUFFERSIZE;

  AV_APS1 = map(AV_APS1, 784, 349, 1, 1024)
  AV_APS2 = map(AV_APS2, 784, 349, 1024, 1)
  AV_BPS1 = map(AV_BPS1, 784, 349, 1, 1024)
  AV_BPS2 = map(AV_BPS2, 784, 349, 1024, 1)
  // Check for signal shorts to GND or 5V
  if( AV_APS1 < 1 || AV_APS2 < 1 || AV_APS1 > 1024 || AV_APS2 > 1024 
    AV_BPS1 < 1 || AV_BPS2 < 1 || AV_BPS1 > 1024 || AV_BPS2 > 1024){
    ERROR = true;
  }
  else{
    ERROR = false;
  }
  
  // Check for signal plausibility
  if( plausibility_check(AV_APS1, AV_APS2) || plausibility_check(AV_BPS1, AV_BPS2)){
    ERROR = true;
  }
  else{
    ERROR = false;
  }

  if(ERROR){
    THROTTLE = 0;
    BRAKE = 0;
  }
  else{
    THROTTLE = map(AV_APS1, 1, 1024, 0, 255)/map(AV_APS2, 1, 1024, 0, 255);
    THROTTLE = constrain(THROTTLE, 0, 255);
    BRAKE = map(AV_BPS1, 1, 1024, 0, 255)/map(AV_BPS2, 1, 1024, 0, 255);
    BRAKE = constrain(BRAKE, 0, 255);
  }

  // Prevent BSPD from triggering
  // - 5kW from 66kW is 7.5%, so 19 of 255.
  // - 50% of brake is considered 'hard braking' for now.
  if (THROTTLE >= 19 && BRAKE >= 127) {
    THROTTLE = 0;
    BRAKE = 0;        //this means you release the brake??? whouldn't you keep sending the correct value?
  }
  // If time since last packet transmission > DATA_RATE, send new packet:
  if(millis() - last > DATA_RATE){
    Serial.println(THROTTLE);
    Serial.println(BRAKE);

    CAN.beginPacket(0x12);
    CAN.write(THROTTLE);
    CAN.write(BRAKE);
    CAN.write(APS_Error);
    CAN.write(BPS_Error);
    CAN.endPacket();
    // Update timer:
    last=millis();
  }
}



// int plausibility_check(int POT1, int POT2, bool isTPS) {

//   if ((POT1 + (POT1 / 10)) < POT2 || (POT2 + (POT2 / 10)) < POT1) { //if on pot is more than 10% lager or smaller than the other    !! PLEASE CHECK THIS PART !!
//     return 0;
//     if (isTPS) {
//       if (first_TPS_Error) {              //if it is the TPS sensor and the error flag is not yet set
//         firstTPSErrorTime = millis();         //set current time for the first error
//       }
//       if (millis() - firstTPSErrorTime > MAX_DIFF_MS) { //AAAAAAAAAAAAAAHHHH SOMETHING WENT HORRBLY WRONG< STOP THE CAR!!!
//         TPS_Error = true;
//         THROTTLE = 0;
//         BRAKE = 0;
//       }
//     }
//     else {
//       if (first_BPS_Error) {              //if it is the BPS sensor and the error flag is not yet set
//         firstBPSErrorTime = millis();         //set current time for the first error
//       }
//       if (millis() - firstBPSErrorTime > MAX_DIFF_MS) { //AAAAAAAAAAAAAAHHHH SOMETHING WENT HORRBLY WRONG< STOP THE CAR!!!
//         BPS_Error = true;
//         THROTTLE = 0;
//         BRAKE = 0;
//       }
//     }
//   }

//   else {    //everything is doing fine
//     return (POT1 + POT2) / 2;     //return average from both POTS

//     if (isTPS) {
//       first_TPS_Error = true;       //reset flag
//       TPS_Error = false;
//     } else {
//       first_BPS_Error = true;       //reset flag
//       BPS_Error = false;
//     }
//   }
// }

bool plausibility_check(long POT1, long POT2) {

  bool IMPLAUSIBLE = true;

  float RATIO = (float)POT1 / (float)POT2;
  float DIFF = abs(RATIO - POT_RATIO);

  if (DIFF > MAX_DIFF) {
    IMPLAUSIBLE = true;
  } else {
    IMPLAUSIBLE = false;
  }
  return IMPLAUSIBLE;
}

