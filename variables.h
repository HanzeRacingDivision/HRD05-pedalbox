#define DATA_RATE 10                /// Update CAN and sensor readings every x ms 

volatile int ReadyToDrive = 0;               /// Dashboard ready to drive status
volatile int TSReady = 0;                    /// Dashboard tractive system status

int APPS1 = 0;                      /// Potentiometer 1 of the accelerator pedal
int APPS2 = 0;                      /// Potentiometer 2 of the accelerator pedal
float APPS = 0;                     /// Holds the average of the two values
const float PEDAL_DEADZONE = 0.5f;         /// Minimum pedal position to activate torque, measured in Nm

// Should probably add a calibration phase during startup to find the offset and diff values
float POT_OFFSET= 79.0f;               /// The physical difference calculated between the accelerator sensors
float MAX_DIFF = 90.0f;                /// Maximum difference is 10% of the 0 to 1023 Arduino values

int BPS1 = 0;
int BPS2 = 0;

float THROTTLE = 0.0f;                 /// Final acceleration torque value after plausibility check
float BRAKE = 0.0f;                    /// Final brake torque value after plausibility check
