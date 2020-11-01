#pragma once

#include <mcp2515.h> // https://github.com/autowp/arduino-mcp2515
#include "./variables.h"
#include "./variables_dmc.h"

MCP2515 mcp2515(10);

namespace CAN
{

    unsigned long last_can_sent;                        // Time in milliseconds since last sent CAN message
    unsigned long last_valid_can_received;              // Time in milliseconds since last received and recognized CAN message

    can_frame MSG;                                      // Generic CAN message to be used by MCP library to fill message

    void setup();
    void get_messages();
    bool should_send_update();

    namespace DASH {
        can_frame DASH_MSG = { 0x036, 2 };
    }

    namespace DMC514 {
        can_frame DMC_CTRL = { 0x210, 8 };               // Brusa DMC514: Enable power and manage control modes
        can_frame DMC_TRQS = { 0x258, 8 };               // Brusa DMC514: Read ready state, torque, speed, etc.
        can_frame DMC_LIM  = { 0x211, 8 };               // Brusa DMC514: Voltage and current limits
        can_frame DMC_ACTV = { 0x258, 8 };               // Brusa DMC514: Actual DC voltage and current 
        can_frame DMC_TEMP = { 0x458, 8 };               // Brusa DMC514: Inverter and motor temperatures
        can_frame DMC_ERR  = { 0x25A, 8 };               // Brusa DMC514: Errors (described in Brusa documentation) 

        void set_standby(const bool& clearError = false);
        void send_DMC_CTRL(float THROTTLE = 0.0f, float BRAKE = 0.0f);
    }

}
