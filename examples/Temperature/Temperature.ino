/*
 * Temperature
 * Reads the on-board LMT84 temperature sensor once a second and prints the
 * ADC reading, the signal voltage and the temperature in °C, °F and K.
 *
 * Jumpers: JP1 at U1 (on-board sensor), JP2 at "MCU ADC", JP3 at 3.3 V.
 * Change SOURCE to TempSource::SENSOR to read the sensor before the
 * conditioning stage. TempSource::EXT is meant for an external ADC: on the
 * board's 3.3 V ADC it only reads above about 21 °C.
 *
 * Each reading averages SAMPLES ADC reads at 12-bit resolution, which steadies
 * the value. Every column is its own reading, so they can differ slightly.
 */
#include <AnalogShield.h>

AnalogShield shield;

const TempSource SOURCE = TempSource::MCU;
const uint8_t SAMPLES = 16;

void setup() {
  Serial.begin(115200);
  shield.begin();
  shield.SetAdcResolution(12);  // stays at 10 bits on an Uno R3
}

void loop() {
  Serial.print("Raw: ");
  Serial.print(shield.TempRaw(SOURCE, SAMPLES));
  Serial.print("  ");
  Serial.print(shield.TempMillivolts(SOURCE, SAMPLES));
  Serial.print(" mV  ");
  Serial.print(shield.TempCelsius(SOURCE, SAMPLES), 1);
  Serial.print(" °C  ");
  Serial.print(shield.TempFahrenheit(SOURCE, SAMPLES), 1);
  Serial.print(" °F  ");
  Serial.print(shield.TempKelvin(SOURCE, SAMPLES), 1);
  Serial.println(" K");

  delay(1000);
}
