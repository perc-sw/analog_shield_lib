/**
 * @file test_support.h
 * @brief Included by every test file: doctest, the fake Arduino API, the library.
 *
 * Use TEST_CASE_FIXTURE(HostTest, "...") so each test starts from power-on pin state.
 */
#pragma once

#include <doctest/doctest.h>

#include "AnalogShield.h"
#include "Arduino.h"

struct HostTest {
  HostTest() { fake::reset(); }
};
