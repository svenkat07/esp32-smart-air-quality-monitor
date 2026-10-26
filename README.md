# ESP32 Environmental Monitor

**Software-only reconstruction, October 2026. AI-assisted implementation.**
This repository rebuilds the project concept described in my CV after the original
code, figures and measurements were lost. It is new work, not recovered evidence.
The native C++ simulator and logic tests run without hardware. ESP32 firmware is
provided as a hardware integration draft and has not been compiled or tested on a board.
No physical sensor measurements, cloud deployment, calibration or performance claims
are presented. All readings in `examples/` are explicitly simulated.

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

Do not describe the generated simulation as original experimental results. Review,
understand and extend this AI-assisted implementation before discussing it in interviews.

## Primary references

- ESP32 ADC: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html
- FreeRTOS queues: https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/main/ch05.md
- PubSubClient: https://pubsubclient.knolleary.net/api
- DHT driver: https://github.com/adafruit/DHT-sensor-library

The repository contains newly authored source and documentation. No external sensor
datasets or borrowed experimental figures are included.
