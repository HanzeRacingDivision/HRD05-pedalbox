#include <mcp2515.h>                                    /// https://github.com/autowp/arduino-mcp2515
MCP2515 mcp2515(10);

unsigned long last_can;                                 /// Holds time in ms.

struct can_frame MSG;                                   /// Generic CAN message
struct can_frame DASH_MSG = { 0x036, 2 };               /// Our specified dashboard message

void CAN_setup() {
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  last_can = millis();
}

void CAN_update() {
  if (mcp2515.readMessage(&MSG) == MCP2515::ERROR_OK){      /// If there are no errors, start reading message pointer
    
    if(DASH_MSG.can_id == MSG.can_id) {
      ReadyToDrive = MSG.data[0];                           /// Read RTD button from the Dashboard
      TSReady      = MSG.data[1];                           /// Read TS status from the Dashboard
    }

    if(DMC_TRQS.can_id == MSG.can_id) {
      DMC_Ready = MSG.data[0];
      DMC_SpdAct = MSG.data[6];
    }
  }
}

void send_DMC_standby(bool clearError = false) {
  
  DMC_ClrError = clearError;                      /// If there was an error, this will be 1. 
  DMC_SpdRq = 0;
  DMC_TrqRq = 0;
  DMC_EnableRq = 0;

  byte bitData[8];

  bitData[0] = DMC_EnableRq;
  bitData[1] = DMC_ModeRq;
  bitData[2] = DMC_OscLimEnableRq;
  bitData[3] = 0;
  bitData[4] = DMC_ClrError;
  bitData[5] = 0;
  bitData[6] = DMC_NegTrqSpd;
  bitData[7] = DMC_PosTrqSpd;

  DMC_CTRL.data[0] = bitData;
  DMC_CTRL.data[2] = 0;                           /// DMC_SpdRq
  DMC_CTRL.data[3] = 0;                           /// DMC_SpdRq
  DMC_CTRL.data[4] = 0;                           /// DMC_TrqRq
  DMC_CTRL.data[5] = 0;                           /// DMC_TrqRq
  
  mcp2515.sendMessage(&DMC_CTRL);
  last_can = millis();

  if(DMC_ClrError) DMC_ClrError = 0;              /// Reset error clear bit after sending clear request
}

void send_DMC_CTRL(int throttle = 0, int enable = 0){

  DMC_TrqRq = throttle;
  DMC_EnableRq = enable;

  byte bitData[8];

  bitData[0] = DMC_EnableRq;
  bitData[1] = DMC_ModeRq;
  bitData[2] = DMC_OscLimEnableRq;
  bitData[3] = 0;
  bitData[4] = DMC_ClrError;
  bitData[5] = 0;
  bitData[6] = DMC_NegTrqSpd;
  bitData[7] = DMC_PosTrqSpd;

  Serial.println(DMC_CTRL.data[0]);               /// Needs testing

  int DMC_TrqRq = throttle * 100;                 /// Requested torque, 0.01Nm/bit
  
  DMC_CTRL.data[0] = bitData;
  DMC_CTRL.data[2] = highByte(DMC_SpdRq);         /// Motorola format
  DMC_CTRL.data[3] = lowByte(DMC_SpdRq);          /// Motorola format 
  DMC_CTRL.data[4] = highByte(DMC_TrqRq);         /// Motorola format
  DMC_CTRL.data[5] = lowByte(DMC_TrqRq);          /// Motorola format
  
  mcp2515.sendMessage(&DMC_CTRL);
  last_can = millis();
}
