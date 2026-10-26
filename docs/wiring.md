# Proposed wiring: classic ESP32 DevKit / ESP32-WROOM-32

Hardware integration draft, not assembled or electrically verified.
These GPIO assignments are not a wiring reference for every ESP32 variant.

| Component connection | Proposed connection | Note |
| --- | --- | --- |
| DHT22 VCC / GND | 3.3 V / common GND | Verify module pin labels |
| DHT22 DATA | GPIO4 | 10 kOhm pull-up to 3.3 V for bare sensor; some modules include one |
| MQ-135 heater/module VCC | Regulated 5 V | Use supply rated for heater; never drive from GPIO |
| MQ-135 GND | Common GND | All rails share ground |
| MQ-135 analogue output | Divider -> GPIO34 (ADC1) | Never connect a possible 5 V output directly |
| Divider upper resistor | 15 kOhm, AO to ADC node | Proposed scaling factor 10/(15+10) = 0.4 |
| Divider lower resistor | 10 kOhm, ADC node to GND | 5 V nominal AO -> 2 V ADC nominal |
| ADC filter capacitor | 100 nF, ADC node to GND | Optional starting point; validate settling/loading |
| External indicator | GPIO25 -> 330 Ohm -> LED anode; cathode to GND | No buzzer implemented |

The resistor network is a starting design, not a validated analogue front end.
Check actual sensor/module voltage limits, resistor tolerances, source impedance,
loading, ADC attenuation and board absolute limits. Measure the node before attaching
the ESP32. An MQ-135 module's digital threshold pin is not used. The ADC voltage in
telemetry is the divided node voltage, not the original module voltage.

**Interface correction:** DHT22 uses its own timed digital single-data-wire protocol,
not Dallas 1-Wire, I2C or SPI. MQ-135 is read through ADC. No I2C/SPI peripheral is
implemented in this reconstruction. Those buses should be listed as separate skills
unless a real additional peripheral is integrated and tested.

An MQ-135 requires sensor-specific conditioning, warm-up, baseline characterisation
and calibration. Temperature/humidity and cross-sensitivity affect interpretation.
This repository deliberately avoids inferring individual gas concentrations.
