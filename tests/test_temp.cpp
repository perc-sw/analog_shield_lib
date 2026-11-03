/**
 * @file test_temp.cpp
 * @brief Temperature signal reads: TempSource selects the analog pin; conversion to °C.
 */
#include <cmath>
#include <initializer_list>

#include "test_support.h"

using namespace AnalogShieldPins;

TEST_CASE_FIXTURE(HostTest, "TempRaw() reads the pin for each TempSource") {
  AnalogShield shield;
  shield.begin();
  fake::pins[SENSOR_IN].analog_value = 100;
  fake::pins[TEMP_SIG_MCU].analog_value = 200;
  fake::pins[TEMP_SIG_EXT].analog_value = 300;
  CHECK(shield.TempRaw(TempSource::SENSOR) == 100);
  CHECK(shield.TempRaw(TempSource::MCU) == 200);
  CHECK(shield.TempRaw(TempSource::EXT) == 300);
}

TEST_CASE_FIXTURE(HostTest, "TempMillivolts() scales the 10-bit reading to a 3.3 V reference") {
  AnalogShield shield;
  shield.begin();
  fake::pins[TEMP_SIG_MCU].analog_value = 0;
  CHECK(shield.TempMillivolts(TempSource::MCU) == 0);
  fake::pins[TEMP_SIG_MCU].analog_value = 1023;
  CHECK(shield.TempMillivolts(TempSource::MCU) == 3300);
  fake::pins[TEMP_SIG_MCU].analog_value = 310;  // 310 * 3300 / 1023 = 1000.0
  CHECK(shield.TempMillivolts(TempSource::MCU) == 1000);
}

namespace {

// 10-bit ADC reading for an LMT84 output of sensor_mv on each path (schematic: 4.3x - 638 mV,
// then the R15 / 10K divider for MCU and EXT), rounded to the nearest count. EXT is on A3,
// the Phyduino Pico's 2.048 V MAX11635; SENSOR and MCU are on the 3.3 V RP2040 ADC.
int RawFor(const TempSource source, const double sensor_mv) {
  double mv = sensor_mv;
  if (source != TempSource::SENSOR) {
    mv = (4.3 * sensor_mv - 638.0) * 6.98 / (6.98 + 10.0);
  }
  const double full_scale_mv = source == TempSource::EXT ? 2048.0 : 3300.0;
  return static_cast<int>(std::lround(mv * 1023.0 / full_scale_mv));
}

// LMT84 datasheet Table 3 (°C, mV)
constexpr struct {
  double celsius;
  double mv;
} LMT84_TABLE[] = {{-50, 1299}, {-40, 1247}, {0, 1034}, {25, 898}, {37, 832}, {100, 476}, {150, 183}};

}  // namespace

// TempCelsius() with the pin for source preset to raw
static float CelsiusAt(const TempSource source, const int raw) {
  const uint8_t pin = source == TempSource::MCU ? TEMP_SIG_MCU : source == TempSource::EXT ? TEMP_SIG_EXT : SENSOR_IN;
  fake::pins[pin].analog_value = raw;
  AnalogShield shield;
  return shield.TempCelsius(source);
}

TEST_CASE_FIXTURE(HostTest, "TempCelsius() matches the LMT84 table on every path") {
  for (const auto source : {TempSource::SENSOR, TempSource::MCU, TempSource::EXT}) {
    for (const auto& point : LMT84_TABLE) {
      CAPTURE(static_cast<int>(source));
      CAPTURE(point.celsius);
      // One ADC count is under 0.6 °C on SENSOR, 0.35 °C on MCU and 0.25 °C on EXT
      CHECK(std::fabs(CelsiusAt(source, RawFor(source, point.mv)) - point.celsius) <= 0.5);
    }
  }
}

TEST_CASE_FIXTURE(HostTest, "TempMillivolts() uses each pin's full scale on the Phyduino Pico") {
  AnalogShield shield;
  fake::pins[TEMP_SIG_MCU].analog_value = 1023;
  fake::pins[TEMP_SIG_EXT].analog_value = 1023;
  CHECK(shield.TempMillivolts(TempSource::MCU) == 3300);  // A0, RP2040 ADC
  CHECK(shield.TempMillivolts(TempSource::EXT) == 2048);  // A3, MAX11635
}

TEST_CASE_FIXTURE(HostTest, "TempCelsius() reads the same voltage alike on MCU and EXT") {
  // JP2 routes one divider output to A0 or A3; 1000 mV is 310 counts on A0 and 500 on A3
  CHECK(CelsiusAt(TempSource::EXT, 500) == doctest::Approx(CelsiusAt(TempSource::MCU, 310)).epsilon(0.01));
}

TEST_CASE_FIXTURE(HostTest, "TempFahrenheit() and TempKelvin() convert the TempCelsius() reading") {
  AnalogShield shield;
  fake::pins[TEMP_SIG_MCU].analog_value = RawFor(TempSource::MCU, 898);  // about 25 °C
  const float celsius = shield.TempCelsius(TempSource::MCU);
  CHECK(shield.TempFahrenheit(TempSource::MCU) == doctest::Approx(celsius * 9.0 / 5.0 + 32.0));
  CHECK(shield.TempKelvin(TempSource::MCU) == doctest::Approx(celsius + 273.15));
  // Fixed points: 0 °C = 32 °F = 273.15 K, 100 °C = 212 °F = 373.15 K
  fake::pins[SENSOR_IN].analog_value = RawFor(TempSource::SENSOR, 1034);
  CHECK(std::fabs(shield.TempFahrenheit(TempSource::SENSOR) - 32.0) <= 0.9);  // 0.5 °C
  CHECK(std::fabs(shield.TempKelvin(TempSource::SENSOR) - 273.15) <= 0.5);
  fake::pins[SENSOR_IN].analog_value = RawFor(TempSource::SENSOR, 476);
  CHECK(std::fabs(shield.TempFahrenheit(TempSource::SENSOR) - 212.0) <= 0.9);
  CHECK(std::fabs(shield.TempKelvin(TempSource::SENSOR) - 373.15) <= 0.5);
}
