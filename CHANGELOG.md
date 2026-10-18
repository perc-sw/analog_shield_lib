# Changelog

Format: one section per version, newest first.

## v0.1.0 (2026-10-17)

First release, for the Analog Shield Rev-C. Developed for the Phyduino Pico (RP2040, phyduino_bsp
v0.1.0-alpha.1 or later); also builds for the Arduino Uno R3 and Uno R4 Minima.

### Added
- Arduino library layout (`library.properties`, `src/`, `examples/`, `keywords.txt`).
- `AnalogShieldPins` pin map for every shield signal, using Arduino pin names, from the Rev-C
  schematic.
- `AnalogShield::begin()` / `end()`: pin modes, LEDs off, and interrupt tracking of the encoder
  and its switch.
- Rotary encoder: `EncoderCount()`, `EncoderDetents()`, `SetEncoderCount()` (no argument resets
  to 0) and an optional per-step callback, `SetEncoderCallback()`. `EncoderDetents()` uses the
  PEC11L's 2 counts per click (`COUNTS_PER_DETENT`, `DETENTS_PER_REV`).
- Encoder switch: `SwitchPressed()` and an optional press/release callback, `SetSwitchCallback()`.
- Temperature from the on-board LMT84: `TempRaw()`, `TempMillivolts()`, `TempCelsius()`,
  `TempFahrenheit()` and `TempKelvin()`, through the SENSOR, MCU or EXT signal path. The shield's
  signal conditioning is undone before the LMT84 conversion.
- Potentiometer RV1: `PotRaw()`, `PotMillivolts()` and `PotPercent()`.
- `SetAdcResolution()` / `AdcResolution()`: 8 to 12 bits; millivolt, temperature and
  potentiometer readings follow it.
- Optional `samples` argument on every analog reading, to average several ADC reads.
- LEDs D1 (green) and D2 (red): `LedON()` and `LedToggle()`, handling the active-low wiring.
- `EncoderPolling`, `EncoderInterrupt`, `Temperature`, `SerialPlotter` and `LedFade` examples.
- CI (`.github/workflows/ci.yml`): `arduino-lint`, every example compiled for the Uno R3, Uno R4
  Minima and Phyduino Pico, and PC-side unit tests; board toolchains and Doxygen are cached.
- Doxygen API reference with formatted example pages, published with the `docs/` site to GitHub
  Pages.
- Library ZIP (`extras/make-zip.sh`), attached to each GitHub release by
  `.github/workflows/release.yml`.
- CMake project (`CMakeLists.txt`, `CMakePresets.json`) and PC-side unit tests
  (`tests/`, doctest against a fake Arduino API).

### Known limitations
- Temperature conversions are covered by the unit tests but have not been tested on hardware yet.
- EncoderPolling, EncoderInterrupt and LedFade have been run on a Phyduino Pico; Temperature and
  SerialPlotter have not.
- `EncoderDetents()` and `COUNTS_PER_DETENT` assume the PEC11L encoder; on an encoder without
  clicks they do not correspond to anything physical.
- The EXT channel swings up to about 5 V for an external ADC; read on the board's 3.3 V ADC it
  saturates below about 21 °C.
- The Uno R3 stays at 10-bit ADC resolution.
- Not yet submitted to the Arduino Library Manager.
