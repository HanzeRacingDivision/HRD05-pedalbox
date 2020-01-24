#include <SPI.h>
#include <mcp2515.h>    /// Some other libraries will be included when switching to the ESP32 + Transciever setup.


int led=3;               /// LED will turn on when accelerator signal will be present. It is intended for testing purposes.
int led2=4;              /// LED will turn on when brake signal will be present. It is intended for testing purposes.
int buzzer=5;

float APPS1=A0;             ///Potentiometer 1 of the accelerator pedal
float APPS2=A1;             ///Potentiometer 2 of the accelerator pedal
float APPS[3]={0};          ///Array for storing 2 values to average when mapping the throttle
float APPS_AVERAGE=0;        ///Average of the two values read every 5ms

float BPS=5;

float throttle=0;      ///The final value of the throttle signal after the plausibility check.
float brake=0;         ///The final value of the brake signal after the plausibility check


float POT_RATIO = 0.74;             ///The sensitivity ratio calculeted between the accelerator sensors.
float POT_OFFSET= 100;              ///The physical difference calculated between the accelerator sensors.
float MAX_DIFF = 0.1;              ///The maximum diference in percentage between two sensors
float MAX_DIFF_TRAVEL = 90;         ///Maximum difference is 10% of the 0 to 1023 Arduino values

unsigned long last_read;           ///millis when last loop was executed. Updated every 5ms
unsigned long last_can;            ///millis when last loop was executed. Updated every 10ms
int DATA_RATE = 10;                /// Send CAN frame every 10 ms (and read sensors continuously)

MCP2515 mcp2515(10);

struct can_frame canMsgStandby;         ///Initializing CAN message to the bus. This message sends the Standby state of the inverter
struct can_frame canMsgRTD;             /// This message allows the inverter to read the throttle torque request
struct can_frame canMsgStart;           /// Reading the Dashboard Ready-To-Drive state


void setup() {

  Serial.begin(9600);
  pinMode(led, OUTPUT);         ///Declaring LED as output
  pinMode(led2,OUTPUT);
  SPI.begin();                  ///Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
  pinMode(BPS, INPUT);
  pinMode(buzzer,OUTPUT);
                        
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ);             ///Initializing mcp2515 CAN, speed/bitrate, sets to normal mode
  mcp2515.setNormalMode();

  last_read=millis();
  last_can=millis();

  delay(2000);
  /// Set the inverter to Standby
  canMsgStandby.can_id  = 0x210;
  canMsgStandby.can_dlc = 8;
  canMsgStandby.data[0] = 0;      
  canMsgStandby.data[1] = 0x00;   
  canMsgStandby.data[2] = 0;           
  canMsgStandby.data[3] = 0x00;
  canMsgStandby.data[4] = 0x00;
  canMsgStandby.data[5] = 0x00;
  canMsgStandby.data[6] = 0x00;
  canMsgStandby.data[7] = 0x00;
  mcp2515.sendMessage(&canMsgStandby);
  
  while (1)
      {
        if (mcp2515.readMessage(&canMsgStart) == MCP2515::ERROR_OK){
          Serial.print(canMsgStart.can_id, HEX); // print ID
          Serial.print(" "); 
          Serial.print(canMsgStart.can_dlc, HEX); // print DLC
          Serial.print(" ");
          int b=canMsgStart.data[0];  /// Read RTD button from the Dashboard ESP via CAN
          int t=canMsgStart.data[1];  /// Read TS status from the Dashboard ESP via CAN
          if(b==1 && digitalRead(BPS)==1 && t==1)
            {
              tone(buzzer, 3000, 3000);
              break;
            }
          }
      }

  delay(1000);
}

void fake_setup()
{
  /// Set the inverter to Standby
  canMsgStandby.can_id  = 0x210;
  canMsgStandby.can_dlc = 8;
  canMsgStandby.data[0] = 0;      
  canMsgStandby.data[1] = 0x00;   
  canMsgStandby.data[2] = 0;           
  canMsgStandby.data[3] = 0x00;
  canMsgStandby.data[4] = 0x00;
  canMsgStandby.data[5] = 0x00;
  canMsgStandby.data[6] = 0x00;
  canMsgStandby.data[7] = 0x00;
  mcp2515.sendMessage(&canMsgStandby);
  
  while (1)
      {
        if (mcp2515.readMessage(&canMsgStart) == MCP2515::ERROR_OK){
        int b=canMsgStart.data[0];  /// Read RTD button from the Dashboard ESP via CAN
        int t=canMsgStart.data[1];  /// Read TS status from the Dashboard ESP via CAN
        if(b==1 && digitalRead(BPS)==1 && t==1)
          {
          tone(buzzer, 3000, 3000);
          break;
          }
        }
      }
}

int plausibility_check(long POT1, long POT2) {                ///Plausibility with ratio by dividing. This will depend on how we place the sensors

  int IMPLAUSIBLE = 1;

  float RATIO = POT1 / POT2;                        ///Ratio between the two potentiometers
  float DIFF = abs(RATIO - POT_RATIO);              ///Calculating the absolute difference between them, keeping in mind the defined ratio

  if (DIFF > MAX_DIFF) {
    IMPLAUSIBLE = 1;
  } else {
    IMPLAUSIBLE = 0;
  }
  return IMPLAUSIBLE;                   ///1=IMPLAUSIBLE /// 0=PLAUSIBLE
}

int plausibility_check2(long POT1, long POT2) {           ///Plausibility with ratio by subtraction. This is used when the sensors have an offset relative to each other.

  int IMPLAUSIBLE = 1;
  
  float RATIO=abs(POT1-POT2);
  float DIFF=abs(RATIO-POT_OFFSET);
  
  if (DIFF > MAX_DIFF) {
    IMPLAUSIBLE = 1;
  } else {
    IMPLAUSIBLE = 0;
  }
  return IMPLAUSIBLE;                   ///1=IMPLAUSIBLE /// 0=PLAUSIBLE
}

void loop() {

    if(millis()-last_read>=5){                   ///It will only enter every 5 ms
    APPS1=analogRead(A0);
    APPS2=analogRead(A1);                        ///Reading accelerator values
    APPS[1]=APPS[0];
    APPS[0]=(APPS1+APPS2)/2;                     ///Place the new value in the vector, without move the old one in the next position

    last_read=millis();
    }

    APPS_AVERAGE=(APPS[0]+APPS[1])/2;            /// Will always work, no dividing by 0
    
    if(plausibility_check2(APPS1,APPS2)==0)
      {throttle=map(APPS_AVERAGE, 0, 1023, 0, 33);       /// Mapping the throttle value from 0 to 33 Nm and constrain. The value used for mapping is the average between two different readings
       throttle=constrain(throttle, 0, 33);
      }
    else
      throttle=0;
    
    
    if(throttle>0)
      digitalWrite(led, HIGH);
    else                                          /// Turning on an LED when pressing the accelerator
      digitalWrite(led, LOW);

    brake=digitalRead(BPS);
    if(brake==1)
      digitalWrite(led2, HIGH);
      else                                       /// Turning on an LED when pressing the brake
      digitalWrite(led2,LOW);
    
    

    /* Prevent BSPD from triggering. Don't think this is allowed

    if(throttle >=19 && brake>=127)
    {
      throttle=0;
      brake=0;
    }
    */

    
     /*
      *  if TRQRq is 0, EanableRq should be off
      */


     if(millis() - last_can >= DATA_RATE){                     // If time since last packet transmission > DATA_RATE, send new packet. For now it will only enter the if statement every 10ms
      Serial.print("Throttle setting is: ");
      Serial.println(throttle);
      Serial.print("Brake setting is: ");
      Serial.println(brake);
      canMsgRTD.can_id  = 0x210;
      canMsgRTD.can_dlc = 8;
      canMsgRTD.data[0] = 1;      
      canMsgRTD.data[1] = 0;   
      canMsgRTD.data[2] = 0x00;           
      canMsgRTD.data[3] = 0x00;
      canMsgRTD.data[4] = 0;
      canMsgRTD.data[5] = 1;
      canMsgRTD.data[6] = 10000;
      canMsgRTD.data[7] = 30;//throttle;
      mcp2515.sendMessage(&canMsgRTD);
      last_can=millis();
     }

    /*if tractive system is off
      {
        fake_setup();  
      }
      */

}
