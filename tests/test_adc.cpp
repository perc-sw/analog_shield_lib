/**
 * @file test_adc.cpp
 * @brief ADC resolution and averaging of samples, shared by every analog reading.
 */
#include "test_support.h"

using namespace AnalogShieldPins;

TEST_CASE_FIXTURE(HostTest, "SetAdcResolution() accepts 8 to 12 bits and sets analogReadResolution()") {
  AnalogShield shield;
  CHECK(shield.AdcResolution() == 10);
  CHECK(shield.SetAdcResolution(12));
  CHECK(shield.AdcResolution() == 12);
  CHECK(fake::analog_resolution == 12);
  CHECK(shield.SetAdcResolution(8));
  CHECK(shield.AdcResolution() == 8);
}

TEST_CASE_FIXTURE(HostTest, "SetAdcResolution() rejects other widths and leaves the resolution alone") {
  AnalogShield shield;
  CHECK_FALSE(shield.SetAdcResolution(7));
  CHECK_FALSE(shield.SetAdcResolution(13));
  CHECK_FALSE(shield.SetAdcResolution(16));
  CHECK(shield.AdcResolution() == 10);
  CHECK(fake::analog_resolution == 10);
}

TEST_CASE_FIXTURE(HostTest, "Millivolts and temperature follow the resolution") {
  AnalogShield shield;
  REQUIRE(shield.SetAdcResolution(12));
  fake::pins[TEMP_SIG_MCU].analog_value = 4095;
  CHECK(shield.TempMillivolts(TempSource::MCU) == 3300);
  fake::pins[TEMP_SIG_MCU].analog_value = 1241;  // 1241 * 3300 / 4095 = 1000.1
  CHECK(shield.TempMillivolts(TempSource::MCU) == 1000);

  // The same voltage gives the same temperature at either resolution
  fake::pins[SENSOR_IN].analog_value = 1112;  // 896.1 mV at 12 bits
  const float at_12 = shield.TempCelsius(TempSource::SENSOR);
  REQUIRE(shield.SetAdcResolution(10));
  fake::pins[SENSOR_IN].analog_value = 278;  // 896.8 mV at 10 bits
  CHECK(shield.TempCelsius(TempSource::SENSOR) == doctest::Approx(at_12).epsilon(0.01));
}

TEST_CASE_FIXTURE(HostTest, "samples averages that many reads, rounding raw readings to nearest") {
  AnalogShield shield;
  fake::pins[SENSOR_IN].analog_sequence = {100, 101};
  CHECK(shield.TempRaw(TempSource::SENSOR, 2) == 101);  // 100.5
  CHECK(fake::analog_reads == 2);

  fake::pins[SENSOR_IN].analog_sequence = {100, 100, 100, 101};
  fake::pins[SENSOR_IN].analog_next = 0;
  CHECK(shield.TempRaw(TempSource::SENSOR, 4) == 100);  // 100.25
  CHECK(fake::analog_reads == 6);

  // 0 counts as 1
  CHECK(shield.TempRaw(TempSource::SENSOR, 0) == 100);
  CHECK(fake::analog_reads == 7);
}

TEST_CASE_FIXTURE(HostTest, "samples averages before converting, keeping fractions of a count") {
  AnalogShield shield;
  fake::pins[POT].analog_sequence = {0, 1023};
  CHECK(shield.PotPercent(2) == doctest::Approx(50.0));
  fake::pins[POT].analog_next = 0;
  CHECK(shield.PotMillivolts(2) == 1650);
}
