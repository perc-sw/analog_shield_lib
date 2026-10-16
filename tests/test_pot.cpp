/**
 * @file test_pot.cpp
 * @brief Potentiometer RV1 on POT (A5): raw reading, millivolts and position.
 */
#include "test_support.h"

using namespace AnalogShieldPins;

TEST_CASE_FIXTURE(HostTest, "PotRaw() and PotMillivolts() read the POT pin") {
  AnalogShield shield;
  fake::pins[POT].analog_value = 310;
  CHECK(shield.PotRaw() == 310);
  CHECK(shield.PotMillivolts() == 1000);  // 310 * 3300 / 1023 = 1000.0
}

TEST_CASE_FIXTURE(HostTest, "PotPercent() spans 0 to 100 at any resolution") {
  AnalogShield shield;
  fake::pins[POT].analog_value = 0;
  CHECK(shield.PotPercent() == doctest::Approx(0.0));
  fake::pins[POT].analog_value = 1023;
  CHECK(shield.PotPercent() == doctest::Approx(100.0));
  fake::pins[POT].analog_value = 512;
  CHECK(shield.PotPercent() == doctest::Approx(50.05).epsilon(0.001));

  REQUIRE(shield.SetAdcResolution(12));
  fake::pins[POT].analog_value = 4095;
  CHECK(shield.PotPercent() == doctest::Approx(100.0));
  CHECK(shield.PotMillivolts() == 3300);
}
