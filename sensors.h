#pragma once

#include "./config.h"
#include "./variables.h"
#include "./filtering.h"

namespace SENSORS {

    unsigned long last_read;    // Millis when last update was executed.

    MovingAverage APPS1 = MovingAverage(APPS1_IN);
    MovingAverage APPS2 = MovingAverage(APPS2_IN);
    MovingAverage BPS1  = MovingAverage(BPS1_IN);
    MovingAverage BPS2  = MovingAverage(BPS2_IN);

    float THROTTLE = 0.0f;      // Final acceleration torque value (after plausibility check)
    float BRAKE    = 0.0f;      // Final brake torque value (after plausibility check)

    void read();
    
    float check_plausibility(const float &POT1, const float &POT2);

    float get_APPS();
    float get_BPS();

}
