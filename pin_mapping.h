#define BUZZER 5                    // Ready To Drive Sound
#define APPS1_IN A0                 // Accelerator pedal sensor #1
#define APPS2_IN A1                 // Accelerator pedal sensor #2

void PIN_setup() {
  pinMode(BPS, INPUT);
  pinMode(BUZZER, OUTPUT);
}
