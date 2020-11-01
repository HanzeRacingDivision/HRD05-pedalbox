/*
 *      __  __                          ____             _                ____  _       _      _
 *     / / / /___ _____  ____  ___     / __ \____ ______(_)___  ____ _   / __ \(_)   __(_)____(_)___  ____
 *    / /_/ / __ `/ __ \/_  / / _ \   / /_/ / __ `/ ___/ / __ \/ __ `/  / / / / / | / / / ___/ / __ \/ __ \
 *   / __  / /_/ / / / / / /_/  __/  / _, _/ /_/ / /__/ / / / / /_/ /  / /_/ / /| |/ / (__  ) / /_/ / / / /
 *  /_/ /_/\__,_/_/ /_/ /___/\___/  /_/ |_|\__,_/\___/_/_/ /_/\__, /  /_____/_/ |___/_/____/_/\____/_/ /_/
 *
 *  Pedalbox code for the HRD05
 *  Mark Oosting, 2020
 *  Armand Micu, 2020
 *
 */

#include <SPI.h>
#include "config.h"                 // Configuration of TUNABLE pedalbox variables
#include "variables.h"              // Global variables
#include "variables_dmc.h"          // Global variables used for the DMC514 inverter
#include "sensors.h"                // APPS / BPS sensor reading and processing
#include "can.h"                    // Our own CAN namespace with message signatures and functions

void setup()
{
    pinMode(BUZZER, OUTPUT);
    Serial.begin(9600);
    SPI.begin();

    CAN::setup();

    delay(1000); // @TODO: wait on the inverter to get ready
}

void loop()
{

    SENSORS::read();
    
    if (car_is_ready()) { // Check if the car is in ready to drive mode. 

        CAN::get_messages();
        
        if (CAN::should_send_update()) { // We're ready to drive, send pedalbox signals via CAN
                
            CAN::DMC514::send_DMC_CTRL(SENSORS::get_APPS(), SENSORS::get_BPS());

        }

    } else {
        
        // The car is not ready yet. The car remains in standby state until car_is_ready() returns true.

        CAN::get_messages(); // Get CAN messages from inverter and dashboard. Contains activity state and RTD request

        if (CAN::should_send_update()) {
            CAN::DMC514::set_standby(); // @TODO: clear error by sending '1' to this function if needed
        }

        Serial.println("Awaiting ready state.");
        // Serial.print("DMC514: ");       Serial.println(DMC_Ready);
        // Serial.print("TS: ");           Serial.println(TSReady);
        // Serial.print("Dash ready: ");   Serial.println(ReadyToDrive);
        // Serial.println();

        // Play ready to drive sound if the car is ready. 
        // @TODO: Use a simple on/off digital output to the buzzer (with a pre-set frequency)
        // @TODO: Look into the blocking nature of tone(). This will currently stop the main loop from running for the entire duration of the sound.
        if (car_is_ready())
            tone(BUZZER, 800, 3000);
    }
}

bool car_is_ready()
{
    // @TODO Needs the brake pedal status as well.
    return (DMC_Ready && ReadyToDrive && TSReady);
}
