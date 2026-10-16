/**
 * @file test_temp.cpp
 * @brief Temperature signal reads: TempSource selects the analog pin.
 */
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
