#pragma once

// CAN BUS
#define DATA_RATE 20                        // Update CAN and sensor readings every x ms 
#define CAN_TIMEOUT 120                     // If no CAN messages are received after a certain time, go into safe state. 

// SIGNALS
#define BUZZER 5                            // Pin: Ready To Drive Sound output
#define APPS1_IN A0                         // Pin: Accelerator pedal sensor #1
#define APPS2_IN A1                         // Pin: Accelerator pedal sensor #2
#define BPS1_IN A2                          // Pin: Accelerator pedal sensor #1
#define BPS2_IN A3                          // Pin: Accelerator pedal sensor #2
#define SENSOR_FILTER_SAMPLES 12            // Number of samples for the moving average filter

// PEDAL TUNING 
#define POT_OFFSET 100                      // The physical offset difference between the sensors, 0-1023
#define POT_MAX_ERROR 50                    // Maximum difference between sensors. Should not be more than 10% of 0-1023 (~100)
#define POT_DEADZONE 20                     // Minimum pedal position to activate torque, 0-1023

// INVERTER / MOTOR SETUP 0
#define MIN_SPEED 400                       // Pre-calculated motor RPM where the vehicle has a speed of about <5 km/h
#define MAX_RPM 10000                       // Maximum RPM of the motor
#define MAX_TORQUE 33.0f                    // Maximum torque request in Nm
#define MAX_BRAKE 33.0f                     // Maximum braking torque request in Nm. Note: regeneration is not actually used (final request to inverter is 0)
#define ENABLE_REGENERATIVE_BRAKING false   // Enable the inverter to use the motor's back-emf to recuperate energy (sent back to the accumulator).
