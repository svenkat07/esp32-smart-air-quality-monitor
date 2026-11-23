# ESP32 Environmental Monitor

ESP32 environmental-monitoring project featuring portable C++ monitoring logic, a FreeRTOS firmware design, DHT22 temperature/humidity acquisition, MQ-135 analogue sampling, and MQTT telemetry.

**Project status:** The monitoring logic has host-side tests and a deterministic simulator. ESP32 firmware integration and physical sensor testing are pending. See [validation details](docs/validation.md).

## Project visualization

![AI-generated ESP32 hardware concept](docs/ESP32%20Air%20Quality%20Monitor%20Workspace.png)

*AI-generated concept visualization. Hardware and on-screen readings are illustrative.*

## What it demonstrates

C++ stateful monitoring logic shared by a host simulator and ESP32 firmware,
FreeRTOS producer/consumer design, ADC sampling, DHT22 acquisition, MQTT telemetry,
sensor fault handling, smoothing, hysteresis, and bounded latest-sample storage.

The prototype uses a 2-second sample period, exponential smoothing (alpha 0.25),
and demonstration voltage/temperature/humidity thresholds. A 60-second application
startup gate exercises the state machine; it is **not** MQ-135 burn-in or calibration.
The MQ-135 output is reported in ADC millivolts, never as calibrated CO2, ppm or AQI.
This is a portfolio prototype, not a gas alarm or health/safety instrument.

## Run locally (no ESP32, account or external service needed)

From the project directory on Linux/macOS with g++ and Python 3:

```sh
mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -Iinclude tests/test_monitor.cpp -o build/test_monitor
./build/test_monitor
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -Iinclude sim/main.cpp -o build/simulator
./build/simulator > examples/simulated_telemetry.jsonl
python3 tools/report.py
```

Open `examples/simulation_report.html` in your browser. It is a static report of
the generated host simulation, not a live cloud dashboard. There are 120 samples
at 2-second logical intervals; the simulator runs them immediately, not in real time.
The scenario contains a gas-voltage excursion, a temperature excursion, recovery,
and a disconnected-temperature-sensor scenario represented by JSON `null`.


## Visual documentation

- [Recruiter project guide](docs/recruiter-guide.md): key source files, design decisions, test instructions and validation boundaries.
- [System architecture diagram](docs/architecture.md): proposed sensor-to-firmware-to-MQTT data flow.
- [Firmware monitoring flowcharts](docs/firmware-flow.md): state transitions, alert hysteresis, fault handling and network task.

## Architecture

`sensorTask` owns DHT/ADC and monitoring state, drives the indicator, then overwrites
a one-element FreeRTOS mailbox. `networkTask` owns Wi-Fi/MQTT and consumes the latest
result. It retries MQTT every 5 seconds and Wi-Fi every 10 seconds. Reconnect calls
can block the network task, but not sensor sampling. Old readings are discarded
after 10 seconds. There is no offline event history. MQTT uses QoS 0, so a successful
publish call does not prove broker delivery. No lossless-delivery claim is made.

| State | Behaviour |
| --- | --- |
| warmup | Valid samples during first 60 seconds; no alarm evaluation |
| normal | All limits clear |
| alert | Smoothed gas >=1600 mV, temperature >=35 C, or humidity >=80% |
| alert recovery | Clear only when gas <=1300 mV, temperature <=32 C and humidity <=75% |
| sensor_fault | Invalid/non-finite DHT values, impossible ranges, or ADC >=3000 mV |

A fault overrides displayed alert state and lights the indicator. The alert latch is
preserved across faults. An open MQ-135 wire cannot reliably be detected by software
alone; zero ADC is not automatically treated as a fault. ADC saturation screening
does not prove sensor integrity. Uptime wraps after about 49.7 days; elapsed-time
checks tolerate wrap, and startup completion remains latched after wrap.

## Planned hardware integration

See [docs/wiring.md](docs/wiring.md). Copy `include/secrets.example.h` to
`include/secrets.h` and configure your private LAN broker and Wi-Fi. Never commit
real credentials. With PlatformIO installed, run `pio run` to build and
`pio run --target upload` only after verifying the actual wiring and board.
Versions are pinned in `platformio.ini` as a proposed build baseline; they were
not downloaded or compiled in this environment.

The draft uses plaintext MQTT on a trusted development LAN with no authentication.
Do not expose it publicly. Add broker authentication, TLS with certificate validation,
device-specific topics, provisioning and backoff before wider deployment.
Local simulator output does not travel over MQTT or Wi-Fi. Firebase is not part of
this project; it belongs to the separate RFID concept in my CV.

## Validation and next steps

See [docs/validation.md](docs/validation.md) for exactly what was run. Before presenting
this as a working physical system: compile the firmware, test DHT timing and ADC
levels, verify task stack usage, simulate broker loss, inspect output waveforms,
and collect new sensor logs with actual timestamps and a documented setup.

## Accurate portfolio wording

“Reconstructed an ESP32 environmental-monitoring prototype with C++ monitoring
logic, FreeRTOS firmware architecture and MQTT telemetry. Validated state transitions
using a deterministic host simulator; hardware integration remains pending.”

## Primary references

- ESP32 ADC: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html
- FreeRTOS queues: https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/main/ch05.md
- PubSubClient: https://pubsubclient.knolleary.net/api
- DHT driver: https://github.com/adafruit/DHT-sensor-library

The repository contains newly authored source and documentation. No external sensor
datasets or borrowed experimental figures are included.
