/*
 * plotter
 * Prints the shield's readings four times a second as label:value pairs, for
 * the Arduino IDE Serial Plotter. Arduino version of the MicroPython
 * examples/plotter/main.py. The board's built-in LED blinks while it runs.
 * Phyduino Pico only.
 *
 * Jumpers: JP1 at Internal (on-board U1), JP2 at Internal (A0) to start,
 * JP3 (VIO) at 3.3 V, JP4 at 5 V.
 * Output: Tools → Serial Plotter at 115200 baud, one line every 250 ms:
 *   RawTemp      raw LMT84 sensor (SENSOR_IN, A1), in °C
 *   Temp         conditioned signal on the selected channel, in °C
 *   EncPosition  encoder clicks (AnalogShield::EncoderDetents())
 *   Channel      selected analog input, 0 for A0, 3 for A3
 *   Pot          RV1 wiper voltage (POT, A5), in V
 *   Vbus         USB VBUS voltage, in V (measured by the MAX11635 on its
 *                channel 3)
 * For example
 *   RawTemp:22.4,Temp:22.1,EncPosition:6,Channel:0,Pot:1.650,Vbus:5.02
 *
 * Pressing the knob switches between A0 and A3. It only tells the code where
 * JP2 is, so move JP2 to match (A3 is JP2 at External). The LEDs show the
 * channel:
 *   green D1        A0 (TEMP_SIG_MCU, RP2040 ADC)
 *   red D2          A3 (TEMP_SIG_EXT, MAX11635 ADC)
 *   both on         the MAX11635 did not respond (until the next press); no
 *                   line is printed for that sample
 *
 * The MAX11635 reads A3, the potentiometer on A5 and VBUS, and returns 0 when
 * it does not respond. VBUS is about 5 V whenever the board is plugged in to
 * USB, so a VBUS reading of 0 means the MAX11635 is not answering, whichever
 * channel is selected.
 *
 * The two temperatures should agree to within about 3 °C. RawTemp is coarser
 * and noisier, as A1 is read with no amplifier.
 *
 * Each reading averages SAMPLES ADC reads at 12-bit resolution, which steadies
 * the value.
 */
#include <AnalogShield.h>
#include <stusb4500.h>  // stusb4500::vbusMillivolts()

using namespace AnalogShieldPins;

static AnalogShield Shield;

constexpr uint8_t SAMPLES = 16;
constexpr uint32_t PRINT_MS = 250;      // time between plotter lines
constexpr uint32_t DEBOUNCE_MS = 50;    // presses closer together than this are switch bounce

// Changed by the switch callback, so volatile
volatile bool use_a3 = false;

// Shows the selected channel on the LEDs: green for A0, red for A3
void showChannel(bool a3) {
  Shield.LedON(LED1, !a3);
  Shield.LedON(LED2, a3);
}

// Runs on every press and release: keep it short, no Serial or delay()
void onSwitch(bool pressed) {
  static uint32_t last_press = 0;

  if (!pressed) {
    return;
  }

  const uint32_t now = millis();

  if (now - last_press >= DEBOUNCE_MS) {
    use_a3 = !use_a3;
    showChannel(use_a3);
  }

  last_press = now;
}

void setup() {
  Serial.begin(115200);
  Shield.begin();
  Shield.SetAdcResolution(12);
  showChannel(use_a3);
  Shield.SetSwitchCallback(onSwitch);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  static uint32_t last_print = 0;

  const uint32_t now = millis();

  if (now - last_print < PRINT_MS) {
    return;
  }

  last_print = now;
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  const bool a3 = use_a3;
  const TempSource channel = a3 ? TempSource::EXT : TempSource::MCU;

  // VBUS is on the MAX11635 too, so 0 V means that ADC did not respond:
  // show the error and skip the line
  const uint32_t vbus_mv = stusb4500::vbusMillivolts();

  if (vbus_mv == 0) {
    Shield.LedON(LED1, true);
    Shield.LedON(LED2, true);
    return;
  }

  const float raw_temp = Shield.TempCelsius(TempSource::SENSOR, SAMPLES);
  const float amp_temp = Shield.TempCelsius(channel, SAMPLES);
  const float pot_volts = static_cast<float>(Shield.PotMillivolts(SAMPLES)) / 1000.f;

  // label:value pairs, comma separated, one line per sample
  Serial.print("RawTemp:");
  Serial.print(raw_temp, 1);
  Serial.print(",Temp:");
  Serial.print(amp_temp, 1);
  Serial.print(",EncPosition:");
  Serial.print(Shield.EncoderDetents());
  Serial.print(",Channel:");
  Serial.print(a3 ? 3 : 0);  // analog input number, A3 or A0
  Serial.print(",Pot:");
  Serial.print(pot_volts, 3);
  Serial.print(",Vbus:");
  Serial.print(static_cast<float>(vbus_mv) / 1000.f, 2);
  Serial.println();
}
