#pragma once

#include "./config.h"
#include "./variables.h"

namespace SENSORS {

    unsigned long last_read;    // Millis when last update was executed.

    int APPS1  = 0;             // Raw accelerator position sensor value 1
    int APPS2  = 0;             // Raw accelerator position sensor value 2
    float APPS = 0.0f;          // Holds the average of the two values

    int BPS1   = 0;             // Raw brake position sensor value 1
    int BPS2   = 0;             // Raw brake position sensor value 2
    float BPS  = 0.0f;          // Holds the average of the two values

    float THROTTLE = 0.0f;      // Final acceleration torque value (after plausibility check)
    float BRAKE = 0.0f;         // Final brake torque value (after plausibility check)

    void read();
    
    float check_plausibility(const float &POT1, const float &POT2);

    float get_APPS();
    float get_BPS();

}
