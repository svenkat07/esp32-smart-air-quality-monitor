# Firmware Monitoring Flow

> **Status:** Documentation of the software prototype and proposed ESP32 integration. Not evidence of hardware testing.

```mermaid
flowchart TD
  A["Start / initialize"] --> B["Sample DHT22 and ADC\nEvery 2 seconds"]
  B --> C{"Invalid DHT / ADC >= 3000 mV?"}
  C -- Yes --> F["sensor_fault\nIndicator on; preserve alert latch"]
  C -- No --> W{"First 60 seconds?"}
  W -- Yes --> U["warmup\nNo alarm evaluation"]
  W -- No --> S["Apply exponential smoothing\nalpha = 0.25"]
  S --> H{"Alert latch active?"}
  H -- No --> T{"Gas >= 1600 mV OR\nTemp >= 35 C OR\nHumidity >= 80%?"}
  T -- Yes --> AL["alert"]
  T -- No --> N["normal"]
  H -- Yes --> R{"Gas <= 1300 mV AND\nTemp <= 32 C AND\nHumidity <= 75%?"}
  R -- Yes --> N
  R -- No --> AL
  F --> Q["Overwrite latest-sample mailbox"]
  U --> Q
  AL --> Q
  N --> Q
  Q --> B
```

## Separate network task

```mermaid
flowchart TD
  Q["Read latest-sample mailbox"] --> ST{"Sample older than 10 s?"}
  ST -- Yes --> DROP["Discard stale reading"]
  ST -- No --> WIFI{"Wi-Fi connected?"}
  WIFI -- No --> WR["Retry Wi-Fi every 10 s"]
  WIFI -- Yes --> MQTT{"MQTT connected?"}
  MQTT -- No --> MR["Retry MQTT every 5 s"]
  MQTT -- Yes --> PUB["Publish latest telemetry\nMQTT QoS 0"]
```

## Important notes

- The 60-second startup gate is a **software state-machine exercise**, not MQ-135 sensor burn-in or calibration.
- Thresholds are demonstration values, not safety limits.
- A fault overrides the displayed alert state but does not reset the alert latch.
- Zero ADC voltage is not automatically classified as an open-wire fault.
- The native simulator advances 120 samples at 2-second **logical** intervals; it is not real-time measurement.
- Refer to [validation.md](validation.md) for what has actually been tested.
