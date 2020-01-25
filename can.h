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
  if (mcp2515.readMessage(&MSG) == MCP2515::ERROR_OK){
    if(DMC_TRQS.can_id == MSG.can_id) {
      DMC_SpdAct = MSG.data[6];
    }
  }
}

void send_DMC_standby() {
  DMC_CTRL.data[0] = 0;
  DMC_CTRL.data[1] = 0x00;
  DMC_CTRL.data[2] = 0;
  DMC_CTRL.data[3] = 0x00;
  DMC_CTRL.data[4] = 0x00;
  DMC_CTRL.data[5] = 0x00;
  DMC_CTRL.data[6] = 0x00;
  DMC_CTRL.data[7] = 0x00;
  mcp2515.sendMessage(&DMC_CTRL);
  last_can = millis();
}

void send_DMC_CTRL(int Torque, int Enable){
  // data_0 = {1,0,0,0,0,0,0,1};
  DMC_CTRL.data[0] = Enable;
  DMC_CTRL.data[2] = 10000;                 // Motorola format, should be split over 2 bytes: data[2] and data[3].
  DMC_CTRL.data[4] = Torque;                // Motorola format, however, since this a max of 33Nm, it fits within 1 byte. 
  mcp2515.sendMessage(&DMC_CTRL);
  last_can = millis();
}
