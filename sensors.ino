#include "./sensors.h"

/**
 * Gets latest sensor values and processes values
 */
void SENSORS::read()
{

    APPS1.update();
    APPS2.update();
    BPS1.update();
    BPS2.update();
    
    last_read = millis();

    Serial.println(APPS1.average);
    Serial.println(APPS2.average);
}

/**
 * Plausibility check, checks if the ratio between the 2 sensors per pedal correspond with the expected ratio. 
 * 
 * @rule T 11.8.9 - Sensor plausibility check 
 * 
 * @todo Should probably add a calibration phase during startup to find the offset and diff values
 * @todo if ONE pedal is implausible, the other pedal should ALSO be shut down. 
 * 
 * @param POT1 Potentiometer 1 value
 * @param POT2 Potentiometer 2 value
 * @return Average of the 2 Potentiometer values if plausible. Otherwise, returns 0.
 */
float SENSORS::check_plausibility(const float &POT1, const float &POT2)
{

    float DIFF = POT1 - POT2;
    float ERROR = abs(DIFF - POT_OFFSET);

    if (ERROR < POT_MAX_ERROR) {
        
        return (POT1 + POT2) / 2.0f; // Take average if the signal is plausible

    } else {
        
        Serial.println("SENSORS ARE NOT PLAUSIBLE");

        return 0.0f; // @rule T 11.8.8 - Shut down power to motors if there is a discrepancy.

    }
}

/**
 * Gets processed APPS (throttle) signal.
 * 
 * @rule T 11.8.12 - The commanded torque for a released pedal must be <= 0 Nm. 
 *
 * @todo Adjust throttle mapping to incorporate POT_OFFSET (so it fully reaches the maximum)
 * 
 * @return Plausibility checked and mapped throttle value based on APPS sensor values.
 */
float SENSORS::get_APPS()
{

    float APPS = SENSORS::check_plausibility(APPS1.average, APPS2.average);

    if (APPS < POT_DEADZONE)
        APPS = 0.0f;
    
    THROTTLE = map(APPS, 0, 1023, 0, MAX_TORQUE * 100) * 0.01; // Hack: multiply and devide by 100 to keep precision (prevent integer truncation)

    return THROTTLE;
}

/**
 * Gets processed BPS (brake) signal.
 *
 * @return Plausibility checked and mapped brake value based on BPS sensor values.
 */
float SENSORS::get_BPS()
{

    // @todo Remove test code
    return 0.0f;

    float BPS = SENSORS::check_plausibility(BPS1.average, BPS2.average);
    BRAKE = map(BPS, 0, 1023, 0, MAX_BRAKE * 100) * 0.01; // Hack: multiply and devide by 100 to keep precision (prevent integer truncation)

    return BRAKE;
}

/**
 * Checks if the brake is actuated or not. 
 *
 * @return true/false
 */
bool SENSORS::is_brake_pressed()
{
    float BPS = SENSORS::check_plausibility(BPS1.average, BPS2.average);

    return (BPS >= 255);
}
