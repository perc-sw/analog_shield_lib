# Changelog

Format: one section per version, newest first.

## Unreleased: v0.1.0

### Added
- Arduino library layout (`library.properties`, `src/`, `examples/`, `keywords.txt`).
- `AnalogShieldPins` pin map taken from the Analog Shield Rev-C schematic.
- `AnalogShield::begin()`: encoder inputs, LEDs off.
- `AnalogShield::TempCelsius()`, `TempFahrenheit()` and `TempKelvin()`: temperature from the
  on-board LMT84, through the SENSOR, MCU or EXT signal path.
- `AnalogShield::PotRaw()`, `PotMillivolts()` and `PotPercent()`: potentiometer RV1.
- `AnalogShield::SetEncoderCount()` (no argument resets to 0) and `EncoderDetents()`:
  clicks of the PEC11L, 2 counts each (`COUNTS_PER_DETENT`, `DETENTS_PER_REV`).
- `AnalogShield::SetAdcResolution()` / `AdcResolution()`: 8 to 12 bits; millivolt,
  temperature and potentiometer readings follow it.
- Optional `samples` argument on every analog reading, to average several ADC reads.
- `ShieldCheck` and `Temperature` examples.
- CI (`.github/workflows/ci.yml`) and GitHub Pages documentation (`docs/`).
- CMake project (`CMakeLists.txt`, `CMakePresets.json`) and PC-side unit tests
  (`tests/`, doctest against a fake Arduino API).
