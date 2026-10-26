# Validation scope

Host tests: compiled with g++ C++17, `-Wall -Wextra -Werror -pedantic` and assertions
enabled. Checks cover warm-up expiry, normal operation, temperature alert assertion,
hysteresis hold/clear, smoothed analogue alert/recovery, invalid humidity, NaN
temperature serialisation to JSON null, ADC range fault, recovery and startup-gate
behaviour across simulated 32-bit uptime wrap.

The host simulator generates 120 synthetic records. The Python report builder parses
every record, verifies simulated=true and the presence of all four states, and builds
a chart from those records. These are repeatable software tests, not sensor validation.

Not tested: embedded compilation, ESP32 scheduling, DHT electrical/timing behaviour,
MQ-135 response/calibration, analogue front end, Wi-Fi reconnects, broker delivery,
FreeRTOS stack sizes, long-run uptime wrap, TLS or cloud integration.

## Suggested hardware acceptance checks

1. Build firmware and resolve toolchain/library differences.
2. Measure all power rails and ADC node limits before connecting the MCU.
3. Verify at least 2-second DHT read spacing and expected sensor ranges.
4. Disconnect DHT and confirm fault indication and JSON null.
5. Stop the broker and confirm acquisition/indicator continue; restart and inspect telemetry.
6. Verify stale samples do not publish after a prolonged outage.
7. Log stack high-water marks and reconnect duration; run a multi-hour soak.
8. Document new physical measurements, firmware revision, environment and limitations.
