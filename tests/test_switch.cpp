/**
 * @file test_switch.cpp
 * @brief Encoder push switch (ENC_SW): polling and the press/release callback.
 */
#include "test_support.h"

using namespace AnalogShieldPins;

namespace {

int switch_calls;
bool switch_pressed;

void recordSwitch(const bool pressed) {
  ++switch_calls;
  switch_pressed = pressed;
}

}  // namespace

TEST_CASE_FIXTURE(HostTest, "SwitchPressed() is true while the active-low switch is held") {
  AnalogShield shield;
  fake::pins[ENC_SW].input_level = HIGH;  // pulled up, released
  shield.begin();
  CHECK_FALSE(shield.SwitchPressed());
  fake::setInput(ENC_SW, LOW);
  CHECK(shield.SwitchPressed());
  fake::setInput(ENC_SW, HIGH);
  CHECK_FALSE(shield.SwitchPressed());
}

TEST_CASE_FIXTURE(HostTest, "begin() attaches a change interrupt to the switch") {
  AnalogShield shield;
  shield.begin();
  CHECK(fake::pins[ENC_SW].isr != nullptr);
  CHECK(fake::pins[ENC_SW].isr_mode == CHANGE);
}

TEST_CASE_FIXTURE(HostTest, "switch callback reports press and release") {
  switch_calls = 0;
  AnalogShield shield;
  fake::pins[ENC_SW].input_level = HIGH;
  shield.begin();
  shield.SetSwitchCallback(recordSwitch);

  fake::setInput(ENC_SW, LOW);
  CHECK(switch_calls == 1);
  CHECK(switch_pressed);

  fake::setInput(ENC_SW, HIGH);
  CHECK(switch_calls == 2);
  CHECK_FALSE(switch_pressed);
}

TEST_CASE_FIXTURE(HostTest, "switch callback skips an interrupt with no change in state") {
  switch_calls = 0;
  AnalogShield shield;
  fake::pins[ENC_SW].input_level = HIGH;
  shield.begin();
  shield.SetSwitchCallback(recordSwitch);
  fake::pins[ENC_SW].isr();  // spurious edge, still released
  CHECK(switch_calls == 0);
}

TEST_CASE_FIXTURE(HostTest, "no switch callback after it is removed or after end()") {
  switch_calls = 0;
  AnalogShield shield;
  fake::pins[ENC_SW].input_level = HIGH;
  shield.begin();
  shield.SetSwitchCallback(recordSwitch);
  shield.SetSwitchCallback(nullptr);
  fake::setInput(ENC_SW, LOW);
  CHECK(switch_calls == 0);

  shield.SetSwitchCallback(recordSwitch);
  shield.end();
  CHECK(fake::pins[ENC_SW].isr == nullptr);
  fake::setInput(ENC_SW, HIGH);
  CHECK(switch_calls == 0);
}
