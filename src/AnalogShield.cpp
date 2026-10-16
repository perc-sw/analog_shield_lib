#include "AnalogShield.h"

using namespace AnalogShieldPins;

void AnalogShield::begin() {
  // Encoder lines have external pull-ups and RC debounce on the shield
  pinMode(ENC_SW, INPUT);
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);

  // LEDs are active low
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}
