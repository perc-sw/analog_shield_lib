/**
 * @file test_pins.cpp
 * @brief Pin map matches the Analog Shield Rev-C schematic (J4 analog, J7 digital header).
 */
#include "test_support.h"

#include <set>

using namespace AnalogShieldPins;

TEST_CASE("analog signals are on the schematic's header pins") {
  CHECK(TEMP_SIG_MCU == A0);
  CHECK(SENSOR_IN == A1);
  CHECK(TEMP_SIG_EXT == A3);
  CHECK(IR_VOUT == A4);
  CHECK(POT == A5);
}

TEST_CASE("digital signals are on the schematic's header pins") {
  CHECK(ENC_SW == 2);
  CHECK(ENC_A == 3);
  CHECK(ENC_B == 4);
  CHECK(LED2 == 5);
  CHECK(LED1 == 6);
}

TEST_CASE("no two signals share a pin") {
  const uint8_t all[] = {TEMP_SIG_MCU, SENSOR_IN, TEMP_SIG_EXT, IR_VOUT, POT,
                         ENC_SW, ENC_A, ENC_B, LED2, LED1};
  CHECK(std::set<uint8_t>(std::begin(all), std::end(all)).size() == std::size(all));
}
