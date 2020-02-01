#include <mcp2515.h>                                    /// https://github.com/autowp/arduino-mcp2515
MCP2515 mcp2515(10);

unsigned long last_can;                                 /// Holds time in ms.

struct can_frame MSG;                                   /// Generic CAN message
struct can_frame DASH_MSG = { 0x036, 2 };               /// Dashboard message

void CAN_setup() {
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  last_can = millis();
}

void CAN_update() {
  if (mcp2515.readMessage(&MSG) == MCP2515::ERROR_OK){  /// If there are no errors, start reading message pointer
    if(DMC_TRQS.can_id == MSG.can_id) {
      DMC_SpdAct = MSG.data[6];
    }
  }
}

void send_DMC_standby() {
  
  DMC_SpdRq = 0;
  DMC_TrqRq = 0;
  DMC_EnableRq = 0;

  /// Set speed and torque request
  DMC_CTRL.data[0] = 0;                           /// For standby mode, all values in the first byte should be 0. 
  DMC_CTRL.data[2] = highByte(DMC_SpdRq);         /// Motorola format
  DMC_CTRL.data[2] = lowByte(DMC_SpdRq);          /// Motorola format 
  DMC_CTRL.data[4] = DMC_TrqRq;                   /// Motorola format, however, since this is 33Nm max, it fits within 1 byte. 
  
  mcp2515.sendMessage(&DMC_CTRL);
  last_can = millis();
}

void send_DMC_CTRL(int throttle, int enable){

  /// Keep track of inverter values globally. Could probably set all of these without providing them as an argument to this function. 
  DMC_SpdRq = max_RPM;
  DMC_TrqRq = throttle;
  DMC_EnableRq = enable;

  int bitData[8];
  
  bitData[0] = DMC_EnableRq;
  bitData[1] = DMC_ModeRq;
  bitData[2] = DMC_OscLimEnableRq;
  bitData[3] = 0;
  bitData[4] = DMC_ClrError;
  bitData[5] = 0;
  bitData[6] = DMC_NegTrqSpd;
  bitData[7] = DMC_PosTrqSpd;

  int DMC_TrqRq = throttle * 100;                 /// Requested torque, 0.01Nm/bit
  
  /// Set speed and torque request
  DMC_CTRL.data[0] = bitData;

  Serial.println(DMC_CTRL.data[0]);
  
  DMC_CTRL.data[2] = highByte(DMC_SpdRq);         /// Motorola format
  DMC_CTRL.data[3] = lowByte(DMC_SpdRq);          /// Motorola format 
  DMC_CTRL.data[4] = highByte(DMC_TrqRq);         /// Motorola format
  DMC_CTRL.data[5] = lowByte(DMC_TrqRq);          /// Motorola format
  
  mcp2515.sendMessage(&DMC_CTRL);
  last_can = millis();
}
