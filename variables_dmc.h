#include <mcp2515.h>                                    /// https://github.com/autowp/arduino-mcp2515

int MIN_SPEED = 400;                                    /// Manually calculated motor RPM where the vehicle has a speed of <5 km/h
int MAX_RPM = 10000;                                    /// Maximum RPM of the E-racer motor

struct can_frame DMC_CTRL = { 0x210, 8 };               /// Brusa DMC514: Enable power and manage control modes
struct can_frame DMC_TRQS = { 0x258, 8 };               /// Brusa DMC514: Read ready state, torque, speed, etc.
struct can_frame DMC_LIM  = { 0x211, 8 };               /// Brusa DMC514: Voltage and current limits
struct can_frame DMC_ACTV = { 0x258, 8 };               /// Brusa DMC514: Actual DC voltage and current 
struct can_frame DMC_TEMP = { 0x458, 8 };               /// Brusa DMC514: Inverter and motor temperatures
struct can_frame DMC_ERR  = { 0x25A, 8 };               /// Brusa DMC514: Errors (described in Brusa documentation) 

/// DMC_CTRL
bool DMC_EnableRq = 0;
bool DMC_ModeRq = 0;
bool DMC_OscLimEnableRq = 0;
bool DMC_ClrError = 0;
bool DMC_NegTrqSpd = 0;
bool DMC_PosTrqSpd = 0;
int DMC_SpdRq = MAX_RPM;
int DMC_TrqRq = 0;

/// DMC_TRQS
int DMC_Ready = 0;                   /// Inverter ready state
int DMC_SpdAct = 0;                  /// Current motor RPM
