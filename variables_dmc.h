#pragma once

// Here we store the currently known values of the DMC514 inverter. 
// Makes it possible to compare these values during the control loop. 
// Note that these values will become out-of-date if the CAN connection is lost. 

// DMC_TRQS - Current state of the inverter
volatile int DMC_Ready = 0;                     // Inverter ready state
volatile int DMC_SpdAct = 0;                    // Current motor RPM (1RPM/bit)

// DMC_CTRL - Control the inverter / send commands
volatile bool DMC_EnableRq = 0;                 // Enables inverter power stage (if possible)
volatile bool DMC_ModeRq = 0;                   // 0 = torque mode, 1 = speed mode. Should always be TORQUE mode. 
volatile bool DMC_OscLimEnableRq = 0;           // See Brusa documentation
volatile bool DMC_ClrError = 0;                 // Request to clear error latch. Can only be done if DMC_EnableRq = 0.
volatile bool DMC_NegTrqSpd = 0;                // If torque mode enabled: 0 = negative speed disabled, 1 = negative speed enabled
volatile bool DMC_PosTrqSpd = 1;                // If torque mode enabled: 0 = positive speed disabled, 1 = positive speed enabled
volatile int DMC_SpdRq = MAX_RPM;               // If torque mode enabled: sets the speed limit. (Could be interesting for a 'pit' mode).
volatile int DMC_TrqRq = 0;                     // Torque request (0.01Nm/bit)
