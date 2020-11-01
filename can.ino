#include "./can.h"

/**
 * Sets up mcp2515 CAN bus. 
 * @TODO: set up message filters. 
 */
void CAN::setup()
{
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();
}

/**
 * Determine if we're ready to send an update based on the configured data rate. 
 */
bool CAN::should_send_update() {
    return (millis() - last_can_sent) >= DATA_RATE;
}

/**
 * Retrieve CAN messages from the bus. 
 * Note that there is always a 'stack' of messages to be read. 
 * readMessage() only retrieves the first message. Therefore, this function should be called continously.
 */
void CAN::get_messages()
{

    // If we haven't received CAN in a while, something must be wrong. Go into safe state!
    if((millis() - last_valid_can_received) >= CAN_TIMEOUT) {
        Serial.println("ERROR: CAN CONNECTION LOST! PEDALBOX SET TO SAFE STATE.");
        TSReady = 0;
        ReadyToDrive = 0;
        DMC_Ready = 0;
        DMC_TrqRq = 0;
    }

    // Read first CAN message on the CAN stack
    if (mcp2515.readMessage(&MSG) == MCP2515::ERROR_OK)
    {

        if (MSG.can_id == CAN::DASH::DASH_MSG.can_id) {
            ReadyToDrive = MSG.data[0];     // RTD button from the dashboard
            TSReady = MSG.data[1];          // TS status from the dashboard
            last_valid_can_received = millis();
        }

        if (MSG.can_id ==  CAN::DMC514::DMC_TRQS.can_id) {
            DMC_Ready = MSG.data[0];
            DMC_SpdAct = MSG.data[6];
            last_valid_can_received = millis();
        }
    }
}

/**
 * Sends a control message to DMC514 inverter. 
 * 
 * @TODO:   Look into "DMC5_ControlConcept_0.3.pdf" -> section 6.1. 
 *          Enabling/disabling the inverter at high speed could cause a large back-emf braking torque (dangerous)
 * 
 * @param THROTTLE  The torque (in Nm) that the driver requested
 * @param BRAKE     The braking torque (in Nm) that the driver requested
 */
void CAN::DMC514::send_DMC_CTRL(float THROTTLE, float BRAKE)
{

    float TORQUE_REQUEST = 0.0f;

    // Re-constrain requested values to make this idiot-proof.
    THROTTLE  = constrain(THROTTLE, 0.0f, MAX_TORQUE);
    BRAKE     = constrain(BRAKE, 0.0f, MAX_BRAKE);

    // Prevent BSPD from triggering (torque request > 5Nm and braking more than 50%)
    if (THROTTLE > 5.0f && (BRAKE > (MAX_BRAKE * 0.5f)))
        THROTTLE = 0;

    // Reduce requested torque by the braking request. Allows the torque request to become negative (regenerative braking).
    TORQUE_REQUEST = THROTTLE - BRAKE;

    if (DMC_SpdAct > MIN_SPEED) {

        // If regenerative braking is disabled, do not allow negative torque request.
        if(!ENABLE_REGENERATIVE_BRAKING) TORQUE_REQUEST = constrain(TORQUE_REQUEST, 0.0f, MAX_TORQUE); 

    } else {
        
        // Prevent backwards driving by only allowing positive torque below MIN_SPEED.
        // Note that this *should* also be handled by the DMC_NegTrqSpd setting. 
        TORQUE_REQUEST = constrain(TORQUE_REQUEST, 0.0f, MAX_TORQUE); 

        // Prevent bucking by disabling inverter control at low speed. Also refer to Brusa control PDF.
        if(TORQUE_REQUEST < 1.0f) {
            DMC_EnableRq = 0;
            TORQUE_REQUEST = 0.0f;
        } else {
            DMC_EnableRq = 1;
        }

    }
    
    // Convert final torque (Nm) to 0.01Nm/bit
    DMC_TrqRq = TORQUE_REQUEST * 100; 

    byte bitData[8];
    bitData[0] = DMC_EnableRq;
    bitData[1] = DMC_ModeRq;
    bitData[2] = DMC_OscLimEnableRq;
    bitData[3] = 0;
    bitData[4] = DMC_ClrError;
    bitData[5] = 0;
    bitData[6] = DMC_NegTrqSpd;
    bitData[7] = DMC_PosTrqSpd;

    // Serial.println(DMC_CTRL.data[0]);    // Needs testing

    DMC_CTRL.data[0] = *bitData;
    DMC_CTRL.data[2] = highByte(DMC_SpdRq); // Motorola format
    DMC_CTRL.data[3] = lowByte(DMC_SpdRq);  // Motorola format
    DMC_CTRL.data[4] = highByte(DMC_TrqRq); // Motorola format
    DMC_CTRL.data[5] = lowByte(DMC_TrqRq);  // Motorola format

    Serial.println("SENDING APPS: ");
    Serial.println(DMC_TrqRq);

    mcp2515.sendMessage(&DMC_CTRL);
    last_can_sent = millis();
}

/**
 * Set the DMC514 inverter to standby mode. 
 * 
 * @TODO: Handle / track inverter errors.
 * 
 * @param clearError requests an error clear to continue operation
 */
void CAN::DMC514::set_standby(const bool& clearError)
{

    DMC_TrqRq = 0;
    DMC_EnableRq = 0;

    byte bitData[8];
    bitData[0] = DMC_EnableRq;
    bitData[1] = DMC_ModeRq;
    bitData[2] = DMC_OscLimEnableRq;
    bitData[3] = 0;
    bitData[4] = clearError;
    bitData[5] = 0;
    bitData[6] = DMC_NegTrqSpd;
    bitData[7] = DMC_PosTrqSpd;

    DMC_CTRL.data[0] = *bitData;
    DMC_CTRL.data[2] = 0;
    DMC_CTRL.data[3] = 0;
    DMC_CTRL.data[4] = 0;
    DMC_CTRL.data[5] = 0;

    mcp2515.sendMessage(&DMC_CTRL);
    last_can_sent = millis();

    // if (clearError)
    //     clearError = 0; 
}
