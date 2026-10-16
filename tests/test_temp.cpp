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

// ADC reading for an LMT84 output of sensor_mv on each path (schematic: 4.3x - 638 mV,
// then the R15 / 10K divider for the MCU), rounded to the nearest count
int RawFor(const TempSource source, const double sensor_mv) {
  double mv = sensor_mv;
  if (source != TempSource::SENSOR) {
    mv = 4.3 * sensor_mv - 638.0;
  }
  if (source == TempSource::MCU) {
    mv *= 6.98 / (6.98 + 10.0);
  }
  return static_cast<int>(std::lround(mv * 1023.0 / 3300.0));
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

TEST_CASE_FIXTURE(HostTest, "TempCelsius() matches the LMT84 table on the SENSOR and MCU paths") {
  for (const auto source : {TempSource::SENSOR, TempSource::MCU}) {
    for (const auto& point : LMT84_TABLE) {
      CAPTURE(static_cast<int>(source));
      CAPTURE(point.celsius);
      // One ADC count is under 0.6 °C on SENSOR and 0.35 °C on MCU
      CHECK(std::fabs(CelsiusAt(source, RawFor(source, point.mv)) - point.celsius) <= 0.5);
    }
  }
}

TEST_CASE_FIXTURE(HostTest, "TempCelsius() on the EXT path, within the 3.3 V ADC range") {
  for (const auto& point : LMT84_TABLE) {
    if (RawFor(TempSource::EXT, point.mv) > 1023) {
      continue;  // above 3.3 V, below about 21 °C
    }
    CAPTURE(point.celsius);
    CHECK(std::fabs(CelsiusAt(TempSource::EXT, RawFor(TempSource::EXT, point.mv)) - point.celsius) <= 0.5);
  }
  // Full scale: 3.3 V on EXT is a sensor output of (3300 + 638) / 4.3 = 915.8 mV, about 21.8 °C
  CHECK(CelsiusAt(TempSource::EXT, 1023) == doctest::Approx(21.8).epsilon(0.01));
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
