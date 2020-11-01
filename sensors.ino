#include "./sensors.h"

/**
 * Gets latest sensor values and processes values
 * 
 * @TODO: add a small delay between analogReads to prevent multiplexing issues
 * @TODO: add a moving average to smooth sensor data over ~50-100ms
 */
void SENSORS::read()
{

    APPS1 = analogRead(APPS1_IN);
    APPS2 = analogRead(APPS2_IN);

    BPS1 = analogRead(BPS1_IN);
    BPS1 = analogRead(BPS2_IN);
    
    last_read = millis();

    // Serial.print("APPS1 raw: ");    Serial.println(APPS1);
    // Serial.print("APPS2 raw: ");    Serial.println(APPS2);
    // Serial.println();
}

/**
 * Sensor plausibility check. Compares ratio between the 2 sensors per pedal with the expected ratio. 
 * 
 * @TODO: Should probably add a calibration phase during startup to find the offset and diff values
 *
 * @param POT1 Potentiometer 1 value
 * @param POT2 Potentiometer 2 value
 * @return Average of the 2 Potentiometer values if plausible. Otherwise, returns 0.
 */
float SENSORS::check_plausibility(const float &POT1, const float &POT2)
{

    float DIFF = abs(POT1 - POT2);

    if (DIFF < MAX_DIFF) {
        
        return (POT1 + POT2) / 2.0f;    // Take average if plausible

    } else {
        
        Serial.println("SENSORS ARE NOT PLAUSIBLE :(");

        return 0.0f;                    // Return 0 if there is a discrepancy.

    }
}

/**
 * Get processed APPS (throttle) signal.
 *
 * @return Plausibility checked, mapped and constrained throttle value based on APPS sensor values
 */
float SENSORS::get_APPS()
{

    APPS = SENSORS::check_plausibility(APPS1, APPS2);

    if (APPS < POT_DEADZONE)
        APPS = 0.0f;

    THROTTLE = map(APPS, 0, 1023, 0, MAX_TORQUE);       // Torque request: map the throttle value from 0 to maximum available torque
    THROTTLE = constrain(THROTTLE, 0, MAX_TORQUE);      // Torque request: never exceed the maximum allowable torque

    return THROTTLE;
}

/**
 * Get processed BPS (brake) signal.
 *
 * @return Plausibility checked, mapped and constrained brake value based on BPS sensor values
 */
float SENSORS::get_BPS()
{

    // @TODO: Remove test code
    return 0.0f;

    BPS = SENSORS::check_plausibility(BPS1, BPS2);
    BRAKE = map(BPS, 0, 1023, 0, MAX_BRAKE);     // Torque request: map the throttle value from 0 to maximum available torque
    BRAKE = constrain(THROTTLE, 0, MAX_BRAKE);   // Torque request: never exceed the maximum allowable torque

    return BRAKE;
}
