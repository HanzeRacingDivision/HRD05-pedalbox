#define BUZZER 5                    // Pin: RTD buzzer
#define APPS1_IN A0                 // Pin: Accelerator sensor #1
#define APPS2_IN A1                 // Pin: Accelerator sensor #2

#define DATA_RATE 10                /// Update CAN and sensor readings every x ms 

unsigned long last_read;            /// Millis when last loop was executed.

int ReadyToDrive = 0;               /// Dashboard ready to drive status
int TSReady = 0;                    /// Dashboard tractive system status

int APPS1 = 0;                      /// Potentiometer 1 of the accelerator pedal
int APPS2 = 0;                      /// Potentiometer 2 of the accelerator pedal
float APPS = 0;                     /// Holds the average of the two values

float APPS_RATIO = 0.74;            /// The sensitivity ratio between the accelerator pedal sensors
float POT_OFFSET= 100;              /// The physical difference calculated between the accelerator sensors
float MAX_DIFF = 0.1;               /// The maximum diference in percentage between two sensors
float MAX_DIFF_TRAVEL = 90;         /// Maximum difference is 10% of the 0 to 1023 Arduino values

float BPS = 5;                      /// Brake on/off switch
float THROTTLE = 0;                 /// Final acceleration torque value after plausibility check
float BRAKE = 0;                    /// Final brake torque value after plausibility check

int min_rpm = 400;                  /// Manually calculated motor RPM where the vehicle has a speed of <5 km/h
