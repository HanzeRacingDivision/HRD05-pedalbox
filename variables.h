#define DATA_RATE 10                /// Update CAN and sensor readings every x ms 

int ReadyToDrive = 0;               /// Dashboard ready to drive status
int TSReady = 0;                    /// Dashboard tractive system status

int APPS1 = 0;                      /// Potentiometer 1 of the accelerator pedal
int APPS2 = 0;                      /// Potentiometer 2 of the accelerator pedal
float APPS = 0;                     /// Holds the average of the two values

float POT_OFFSET= 79;              /// The physical difference calculated between the accelerator sensors
float MAX_DIFF_TRAVEL = 90;         /// Maximum difference is 10% of the 0 to 1023 Arduino values

float BPS = 5;                      /// Brake on/off switch
float THROTTLE = 0;                 /// Final acceleration torque value after plausibility check
float BRAKE = 0;                    /// Final brake torque value after plausibility check
