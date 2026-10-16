/**
 * @file test_begin.cpp
 * @brief AnalogShield::begin() pin set-up.
 */
#include "test_support.h"

using namespace AnalogShieldPins;

TEST_CASE_FIXTURE(HostTest, "begin() makes the encoder lines plain inputs (pull-ups are on the shield)") {
  AnalogShield shield;
  shield.begin();
  CHECK(fake::pins[ENC_SW].mode == INPUT);
  CHECK(fake::pins[ENC_A].mode == INPUT);
  CHECK(fake::pins[ENC_B].mode == INPUT);
}

TEST_CASE_FIXTURE(HostTest, "begin() turns the active-low LEDs off") {
  AnalogShield shield;
  shield.begin();
  CHECK(fake::pins[LED1].mode == OUTPUT);
  CHECK(fake::pins[LED2].mode == OUTPUT);
  CHECK(digitalRead(LED1) == HIGH);
  CHECK(digitalRead(LED2) == HIGH);
}

TEST_CASE_FIXTURE(HostTest, "begin() sets the LED pins as outputs before writing them") {
  AnalogShield shield;
  shield.begin();
  CHECK(fake::pins[LED1].mode_at_write == OUTPUT);
  CHECK(fake::pins[LED2].mode_at_write == OUTPUT);
}
