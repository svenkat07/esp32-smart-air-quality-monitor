# Recruiter guide: ESP32 Environmental Monitor

## Project in 30 seconds

This repository demonstrates an **AI-assisted, software-only reconstruction** of an embedded environmental-monitoring concept. It includes portable C++ monitoring logic, an ESP32/Arduino FreeRTOS integration draft, MQTT publishing logic, deterministic simulation, and host-side tests.

**Verification boundary:** Host tests and simulated telemetry are documented. The ESP32 firmware has not been compiled or run on physical hardware. The image in the README is AI-generated concept artwork, not test evidence.

## Where to look first

| Area | File | What to inspect |
| --- | --- | --- |
| Core C++ logic | [include/monitor.hpp](../include/monitor.hpp) | State transitions, smoothing, hysteresis, faults and JSON formatting |
| Firmware draft | [src/main.cpp](../src/main.cpp) | Sensor/network task separation, FreeRTOS mailbox, Wi-Fi/MQTT handling |
| Unit tests | [tests/test_monitor.cpp](../tests/test_monitor.cpp) | Assertions for normal, alert, recovery, sensor faults and uptime wrap |
| Simulation | [sim/main.cpp](../sim/main.cpp) | Deterministic, synthetic input scenarios |
| Architecture | [architecture.md](architecture.md) | Proposed data flow and task responsibilities |
| Firmware flow | [firmware-flow.md](firmware-flow.md) | Monitoring state and network logic |
| Wiring proposal | [wiring.md](wiring.md) | Unverified electrical integration assumptions |
| Validation scope | [validation.md](validation.md) | Tested and untested boundaries |

## Design decisions

**Separate acquisition and networking.** The draft `sensorTask` reads the DHT22 and ADC every two seconds and updates the monitoring state. The lower-priority `networkTask` handles Wi-Fi and MQTT. A one-element FreeRTOS queue holds only the latest reading, so delayed network operations do not create an unbounded backlog.

**Use explicit fault states.** Invalid DHT readings or out-of-range ADC values trigger a fault. The indicator is driven by the monitoring state, not by broker availability.

**Avoid false precision.** MQ-135 data is represented as the measured ADC node voltage in millivolts. No calibrated air-quality index, gas concentration or health warning is claimed.

**Bound telemetry freshness.** The firmware draft discards pending readings older than ten seconds. MQTT QoS 0 is best-effort; a successful publish call does not guarantee broker delivery.

## Reproduce the host-side checks

Requires a C++17 compiler and Python 3 on Linux/macOS.

```sh
mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -Iinclude tests/test_monitor.cpp -o build/test_monitor
./build/test_monitor
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -Iinclude sim/main.cpp -o build/simulator
./build/simulator > examples/simulated_telemetry.jsonl
python3 tools/report.py
```

The generated report is `examples/simulation_report.html`. Its charts show simulated values, not sensor measurements.

## Interview discussion points

- Why a one-element queue was selected instead of accumulating every sample
- How smoothing and hysteresis reduce repeated alert transitions
- How sensor faults are represented separately from threshold alerts
- What can be validated on a host and what requires a real ESP32
- Why MQTT QoS 0 and plaintext LAN messaging are limitations

## Next evidence to collect

Compile the PlatformIO target, verify sensor supply and ADC limits with instruments, test actual DHT read timing, measure task stack margins, exercise Wi-Fi/broker outages, and capture timestamped real-device logs. Record the board, wiring, firmware revision and test conditions before making physical-performance claims.
