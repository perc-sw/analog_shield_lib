/*
 * Temperature
 * Reads the on-board LMT84 temperature sensor once a second and prints the
 * ADC reading, the signal voltage and the temperature in °C, °F and K.
 *
 * Jumpers: JP1 at U1 (on-board sensor), JP2 at "MCU ADC", JP3 at 3.3 V.
 * Output: Serial Monitor at 115200 baud, for example
 *   Raw: 1675  1350 mV  22.4 °C  72.3 °F  295.6 K
 *
 * Change SOURCE to TempSource::SENSOR to read the sensor before the
 * conditioning stage. TempSource::EXT is meant for an external ADC: on the
 * board's 3.3 V ADC it only reads above about 21 °C.
 *
 * Each reading averages SAMPLES ADC reads at 12-bit resolution, which steadies
 * the value. Every column is its own reading, so they can differ slightly.
 */
#include <AnalogShield.h>

AnalogShield Shield;

const TempSource SOURCE = TempSource::MCU;
const uint8_t SAMPLES = 16;

void setup() {
  Serial.begin(115200);
  Shield.begin();
  Shield.SetAdcResolution(12);  // stays at 10 bits on an Uno R3
}

void loop() {
  Serial.print("Raw: ");
  Serial.print(Shield.TempRaw(SOURCE, SAMPLES));
  Serial.print("  ");
  Serial.print(Shield.TempMillivolts(SOURCE, SAMPLES));
  Serial.print(" mV  ");
  Serial.print(Shield.TempCelsius(SOURCE, SAMPLES), 1);
  Serial.print(" °C  ");
  Serial.print(Shield.TempFahrenheit(SOURCE, SAMPLES), 1);
  Serial.print(" °F  ");
  Serial.print(Shield.TempKelvin(SOURCE, SAMPLES), 1);
  Serial.println(" K");

  delay(1000);
}
