/*
 * SerialPlotter
 * Streams the encoder, potentiometer and temperature readings to the Arduino
 * IDE Serial Plotter every 200 ms. The encoder switch selects which
 * conditioned temperature channel is read, MCU or EXT.
 *
 * Jumpers: JP1 at U1 (on-board sensor), JP2 at "MCU ADC", JP3 at 3.3 V.
 * Output: Tools → Serial Plotter at 115200 baud, one line per sample:
 *   Encoder        encoder count, 2 per click (AnalogShield::EncoderCount())
 *   Potentiometer  RV1 wiper voltage, in V
 *   Sensor         raw LMT84 sensor (SENSOR_IN, A1), in °C
 *   Temperature    conditioned signal on the selected channel, in °C
 *   Switch         selected channel, -1 for MCU, 1 for EXT
 * For example
 *   Encoder:12,Potentiometer:1.650,Sensor:22.41,Temperature:22.37,Switch:-1
 *
 * Each press of the encoder switch changes the channel; the switch is
 * debounced in onSwitch(). The LED for the selected channel flashes, 500 ms
 * on and 500 ms off:
 *   MCU (TEMP_SIG_MCU, A0)  green D1, Switch = -1
 *   EXT (TEMP_SIG_EXT, A3)  red D2,   Switch = 1
 * JP2 routes the conditioned signal to only one of A0 and A3, so move it to
 * "external ADC" when selecting EXT; the other channel reads an unconnected
 * pin. EXT is meant for an external ADC: on the board's 3.3 V ADC it only
 * reads above about 21 °C.
 *
 * Each reading averages SAMPLES ADC reads at 12-bit resolution (10 bits on an
 * Uno R3), which steadies the value.
 */
#include <AnalogShield.h>

using namespace AnalogShieldPins;

static AnalogShield Shield;

constexpr uint8_t SAMPLES = 16;
constexpr uint32_t PRINT_MS = 200;      // time between plotter lines
constexpr uint32_t FLASH_MS = 500;      // LED on and off time
constexpr uint32_t DEBOUNCE_MS = 50;    // presses closer together than this are switch bounce

// Changed by the switch callback, so volatile
volatile bool use_ext = false;

// Runs on every press and release: keep it short, no Serial or delay()
void onSwitch(bool pressed) {
  static uint32_t last_press = 0;

  if (!pressed) {
    return;
  }

  const uint32_t now = millis();

  if (now - last_press >= DEBOUNCE_MS) {
    use_ext = !use_ext;
  }

  last_press = now;
}

void setup() {
  Serial.begin(115200);
  Shield.begin();
  Shield.SetAdcResolution(12);
  Shield.SetSwitchCallback(onSwitch);
}

void loop() {
  static uint32_t last_flash = 0;
  static uint32_t last_print = 0;
  static bool led_on = false;

  const uint32_t now = millis();
  const bool ext = use_ext;

  // Flash the LED for the selected channel; keep the other one off
  if (now - last_flash >= FLASH_MS) {
    last_flash = now;
    led_on = !led_on;
  }

  Shield.LedON(LED1, led_on && !ext);
  Shield.LedON(LED2, led_on && ext);

  if (now - last_print >= PRINT_MS) {
    last_print = now;

    const TempSource channel = ext ? TempSource::EXT : TempSource::MCU;

    // label:value pairs, comma separated, one line per sample
    Serial.print("Encoder:");
    Serial.print(Shield.EncoderCount());
    Serial.print(",Potentiometer:");
    Serial.print(static_cast<float>(Shield.PotMillivolts(SAMPLES)) / 1000.f, 3);
    Serial.print(",Sensor:");
    Serial.print(Shield.TempCelsius(TempSource::SENSOR, SAMPLES), 2);
    Serial.print(",Temperature:");
    Serial.print(Shield.TempCelsius(channel, SAMPLES), 2);
    Serial.print(",Switch:");
    Serial.println(ext ? 1 : -1);
  }
}
