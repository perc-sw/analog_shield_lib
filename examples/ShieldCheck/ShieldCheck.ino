/*
 * ShieldCheck
 * Blinks the two shield LEDs and prints the analog inputs and encoder lines
 * once a second. Use it to confirm a newly assembled Analog Shield works.
 */
#include <AnalogShield.h>

using namespace AnalogShieldPins;

AnalogShield shield;

void setup() {
  Serial.begin(115200);
  shield.begin();
}

void loop() {
  static bool on = false;
  on = !on;
  digitalWrite(LED1, on ? LOW : HIGH);
  digitalWrite(LED2, on ? HIGH : LOW);

  Serial.print("SENSOR_IN=");
  Serial.print(analogRead(SENSOR_IN));
  Serial.print(" TEMP_SIG_MCU=");
  Serial.print(analogRead(TEMP_SIG_MCU));
  Serial.print(" IR_VOUT=");
  Serial.print(analogRead(IR_VOUT));
  Serial.print(" POT=");
  Serial.print(analogRead(POT));
  Serial.print(" ENC A/B/SW=");
  Serial.print(digitalRead(ENC_A));
  Serial.print(digitalRead(ENC_B));
  Serial.println(digitalRead(ENC_SW));

  delay(1000);
}
