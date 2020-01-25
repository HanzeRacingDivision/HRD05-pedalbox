#define buzzer 5

#define APPS1_IN A0
#define APPS2_IN A1

float APPS1 = 0;        /// Potentiometer 1 of the accelerator pedal
float APPS2 = 0;        /// Potentiometer 2 of the accelerator pedal
float APPS = 0;         /// Average of the two values

float BPS = 5;
float throttle = 0;                 /// The final value of the throttle signal after the plausibility check.
float brake = 0;                    /// The final value of the brake signal after the plausibility check

float POT_RATIO = 0.74;             /// The sensitivity ratio calculeted between the accelerator sensors.
float POT_OFFSET= 100;              /// The physical difference calculated between the accelerator sensors.
float MAX_DIFF = 0.1;               /// The maximum diference in percentage between two sensors
float MAX_DIFF_TRAVEL = 90;         /// Maximum difference is 10% of the 0 to 1023 Arduino values

int motor_rpm = 0;                  /// Motor RPM 
int min_speed = 400;                 /// Manually calculated motor RPM where the vehicle has a speed of 5 km/h

unsigned long last_read;            /// Millis when last loop was executed.
int DATA_RATE = 10;                 /// Send CAN frame every 10 ms (and read sensors continuously)
