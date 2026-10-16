/**
 * @file test_encoder.cpp
 * @brief Rotary encoder counting through the ENC_A / ENC_B interrupts.
 */
#include <initializer_list>

#include "test_support.h"

using namespace AnalogShieldPins;

namespace {

// One full quadrature cycle (4 edges) with A leading B
void stepForward() {
  fake::setInput(ENC_A, HIGH);
  fake::setInput(ENC_B, HIGH);
  fake::setInput(ENC_A, LOW);
  fake::setInput(ENC_B, LOW);
}

// One full quadrature cycle (4 edges) with B leading A
void stepBackward() {
  fake::setInput(ENC_B, HIGH);
  fake::setInput(ENC_A, HIGH);
  fake::setInput(ENC_B, LOW);
  fake::setInput(ENC_A, LOW);
}

}  // namespace

TEST_CASE_FIXTURE(HostTest, "begin() attaches change interrupts to both encoder channels") {
  AnalogShield shield;
  shield.begin();
  CHECK(fake::pins[ENC_A].isr != nullptr);
  CHECK(fake::pins[ENC_B].isr != nullptr);
  CHECK(fake::pins[ENC_A].isr_mode == CHANGE);
  CHECK(fake::pins[ENC_B].isr_mode == CHANGE);
}

TEST_CASE_FIXTURE(HostTest, "encoder counts every edge, up when A leads B") {
  AnalogShield shield;
  shield.begin();
  CHECK(shield.EncoderCount() == 0);
  stepForward();
  CHECK(shield.EncoderCount() == 4);
  stepForward();
  CHECK(shield.EncoderCount() == 8);
}

TEST_CASE_FIXTURE(HostTest, "encoder counts down when B leads A") {
  AnalogShield shield;
  shield.begin();
  stepBackward();
  CHECK(shield.EncoderCount() == -4);
}

TEST_CASE_FIXTURE(HostTest, "encoder returns to zero after turning back") {
  AnalogShield shield;
  shield.begin();
  stepForward();
  stepForward();
  stepBackward();
  stepBackward();
  CHECK(shield.EncoderCount() == 0);
}

TEST_CASE_FIXTURE(HostTest, "end() detaches the encoder interrupts") {
  AnalogShield shield;
  shield.begin();
  shield.end();
  CHECK(fake::pins[ENC_A].isr == nullptr);
  CHECK(fake::pins[ENC_B].isr == nullptr);
  stepForward();
  CHECK(shield.EncoderCount() == 0);
}

namespace {

int callback_calls;
int32_t callback_count;
int8_t callback_step;

void recordStep(const int32_t count, const int8_t step) {
  ++callback_calls;
  callback_count = count;
  callback_step = step;
}

}  // namespace

TEST_CASE_FIXTURE(HostTest, "encoder callback gets the new count and step on every edge") {
  callback_calls = 0;
  AnalogShield shield;
  shield.begin();
  shield.SetEncoderCallback(recordStep);

  fake::setInput(ENC_A, HIGH);
  CHECK(callback_calls == 1);
  CHECK(callback_count == 1);
  CHECK(callback_step == 1);

  fake::setInput(ENC_A, LOW);
  CHECK(callback_calls == 2);
  CHECK(callback_count == 0);
  CHECK(callback_step == -1);
}

TEST_CASE_FIXTURE(HostTest, "encoder keeps counting after the callback is removed") {
  callback_calls = 0;
  AnalogShield shield;
  shield.begin();
  shield.SetEncoderCallback(recordStep);
  shield.SetEncoderCallback(nullptr);
  stepForward();
  CHECK(callback_calls == 0);
  CHECK(shield.EncoderCount() == 4);
}

TEST_CASE_FIXTURE(HostTest, "SetEncoderCount() sets the count, and with no argument resets it") {
  AnalogShield shield;
  shield.begin();
  stepForward();
  shield.SetEncoderCount(100);
  CHECK(shield.EncoderCount() == 100);
  CHECK(fake::interrupts_on);  // interrupts back on afterwards
  stepForward();
  CHECK(shield.EncoderCount() == 104);  // counting carries on from the new value
  shield.SetEncoderCount();
  CHECK(shield.EncoderCount() == 0);
}

TEST_CASE_FIXTURE(HostTest, "EncoderDetents() counts clicks, 2 steps each, rounded toward zero") {
  AnalogShield shield;
  shield.begin();
  for (const int32_t count : {0, 1, 2, 3, 4, 60}) {
    shield.SetEncoderCount(count);
    CHECK(shield.EncoderDetents() == count / 2);
    shield.SetEncoderCount(-count);
    CHECK(shield.EncoderDetents() == -(count / 2));
  }
  shield.SetEncoderCount();
  stepForward();  // one quadrature cycle = 2 clicks
  CHECK(shield.EncoderDetents() == 2);
  CHECK(AnalogShield::COUNTS_PER_DETENT * AnalogShield::DETENTS_PER_REV == 60);
}
