# Changelog

Format: one section per version, newest first.

## Unreleased: v0.1.0

### Added
- Arduino library layout (`library.properties`, `src/`, `examples/`, `keywords.txt`).
- `AnalogShieldPins` pin map taken from the Analog Shield Rev-C schematic.
- `AnalogShield::begin()`: encoder inputs, LEDs off.
- `ShieldCheck` example.
- CI (`.github/workflows/ci.yml`) and GitHub Pages documentation (`docs/`).
- CMake project (`CMakeLists.txt`, `CMakePresets.json`) and PC-side unit tests
  (`tests/`, doctest against a fake Arduino API).
