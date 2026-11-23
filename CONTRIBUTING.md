# Development and validation guidelines

This repository is a portfolio-oriented, AI-assisted software reconstruction. Contributions should be technically reviewable and distinguish **simulation**, **firmware draft**, and **hardware-validated** results.

## Before submitting changes

1. Keep core monitoring logic portable and testable without ESP32 hardware.
2. Add or update host tests when changing alert thresholds, smoothing, hysteresis, fault handling or JSON output.
3. Run the commands in [README.md](README.md#run-locally-no-esp32-account-or-external-service-needed).
4. Document any assumptions, especially pinout, electrical limits, MQTT reliability and sensor calibration.
5. Never commit credentials or claim simulated output is real sensor data.

## Evidence labels

- **Host-tested:** Include reproducible commands and actual test output.
- **Simulated:** Identify synthetic input and generated outputs.
- **Firmware draft:** Say when firmware compilation or board execution has not been verified.
- **Hardware-validated:** Only use after documenting board model, wiring, instruments, firmware revision, timestamps, observations and limitations.

## Code review checklist

- [ ] No secrets or personal credentials
- [ ] Clear distinction between ADC millivolts and calibrated gas measurements
- [ ] Test coverage for changes to monitoring behavior
- [ ] Documentation updated for interface or wiring changes
- [ ] No unsupported reliability, calibration, or physical validation claims
