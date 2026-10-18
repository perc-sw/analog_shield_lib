# Analog Shield Library

[![CI](https://github.com/perc-sw/analog_shield_lib/actions/workflows/ci.yml/badge.svg)](https://github.com/perc-sw/analog_shield_lib/actions/workflows/ci.yml)
[![Version](https://img.shields.io/github/v/release/perc-sw/analog_shield_lib?include_prereleases&sort=semver&label=version)](https://github.com/perc-sw/analog_shield_lib/releases)
![Licence](https://img.shields.io/badge/licence-GPL--3.0-blue)

Arduino library and examples for the [Analog Shield](https://github.com/perc-sw/analog_shield)
used in the University of Toronto, Department of Physics,
"[Introduction to Electronics](https://plrs.physics.utoronto.ca/course-listing/)" course.

The library uses only the standard Arduino API. It is developed for the
[Phyduino Pico](https://github.com/perc-sw/phyduino_pico) (board package
[phyduino_bsp](https://github.com/perc-sw/phyduino_bsp) v0.1.0-alpha.1 or later) and also builds
for the Arduino Uno.

Documentation: <https://perc-sw.github.io/analog_shield_lib/>

## Install

Download `AnalogShield-<version>.zip` from the latest
[release](https://github.com/perc-sw/analog_shield_lib/releases/latest), then in the Arduino IDE use
**Sketch → Include Library → Add .ZIP Library…**.
Examples appear under **File → Examples → AnalogShield**.

To build the ZIP from a local checkout, run `extras/make-zip.sh` (writes `build/AnalogShield-<version>.zip`
from the last commit; pass a tag or commit to package that instead).

## Examples

Open them from **File → Examples → AnalogShield**. Each sketch's header comment lists the jumper
settings it needs and what it prints.

| Example | Shows | Output |
|---|---|---|
| `EncoderPolling` | Reading the encoder count and switch from `loop()` | Serial Monitor |
| `EncoderInterrupt` | The same, with encoder and switch callbacks | Serial Monitor |
| `Temperature` | LMT84 temperature as an ADC reading, mV, °C, °F and K | Serial Monitor |
| `SerialPlotter` | Encoder, potentiometer and temperatures live; the switch selects the MCU or EXT channel, shown on D1/D2 | Serial Plotter |
| `LedFade` | Fading D1 and D2 in and out with PWM (`analogWrite()`) | LEDs only |

Examples that print use 115200 baud.

## Pin map

| Pin | Name | Signal |
|---|---|---|
| A0 | `TEMP_SIG_MCU` | Conditioned temperature signal (JP2 → MCU ADC) |
| A1 | `SENSOR_IN` | Raw temperature sensor (JP1 selects on-board U1 or external J1) |
| A3 | `TEMP_SIG_EXT` | Conditioned temperature signal (JP2 → external ADC) |
| A4 | `IR_VOUT` | Sharp GP2Y0A21YK0F IR distance sensor (J2) |
| A5 | `POT` | Potentiometer RV1 |
| 2 | `ENC_SW` | Rotary encoder switch, active low |
| 3 | `ENC_A` | Rotary encoder A, active low |
| 4 | `ENC_B` | Rotary encoder B, active low |
| 5 | `LED2` | Red LED D2, active low |
| 6 | `LED1` | Green LED D1, active low |

Set JP3 (VDD) to 3.3 V when using a 3.3 V board such as the Phyduino Pico.

## Repository layout

```
library.properties   Arduino library metadata
keywords.txt         IDE syntax colouring
src/                 Library source
examples/            Example sketches (each compiled by CI)
tests/               PC-side unit tests (doctest + fake Arduino API)
CMakeLists.txt       CMake project: library target and tests (not used by the Arduino IDE)
docs/                GitHub Pages site (reference material)
.github/workflows/   CI
```

## Tests

PC-side unit tests compile the real `src/` code against a fake `Arduino.h`
(`tests/fake_arduino/`) that records pin modes and levels. From the repo root:

```
cmake --workflow --preset host-tests
```

Needs a C++17 compiler (GCC or Clang; on Windows, MSYS2 UCRT64), CMake 3.21 or later, Ninja
and internet access the first time (doctest is downloaded at configure time).
Add tests as `tests/test_*.cpp`; they are picked up automatically.
