# Air-Sense

An ESP32-C3 air-quality monitoring node that measures **temperature**,
**humidity**, and **gas concentration**, averages each batch of readings, pushes
the results to the **Blynk** IoT cloud, raises an alert when air quality is bad,
and then enters a **true deep sleep** with the sensors fully powered off.

Built as an Embedded Systems semester project.

---

## Features

- 🌡️ **DHT11** temperature & humidity sensing
- 💨 Analog **gas sensor** with oversampling to smooth ADC noise
- 📊 Batches **5 samples**, uploads the **mean** of each metric to Blynk
- 🚨 Automatic **gas alert** (Blynk event) above a configurable threshold
- 📶 **WiFiManager** captive portal — no hard-coded WiFi credentials
- 🧵 **FreeRTOS** producer/consumer design (sensor task ↔ upload task) with a
  binary semaphore
- 🔋 **MOSFET-switched power rail** for genuinely low deep-sleep current
- 🔐 Credentials isolated in a gitignored `secrets.h`

---

## How the power-down deep sleep works

A standard ESP32 deep sleep only puts the **microcontroller** to sleep — any
sensors wired to the 3V3 rail keep drawing current the whole time. The DHT11 and
gas sensor (especially heated gas sensors) can dominate the sleeping power
budget.

Air-Sense avoids this with a **high-side power switch**:

```
            POWER_PIN (GPIO10)
                  │
                  ▼  gate
   3V3 ──────[ MOSFET ]────── VCC of DHT11 + gas sensor
```

- `POWER_PIN` **HIGH** → MOSFET conducts → sensors powered.
- `POWER_PIN` **LOW**  → MOSFET off → sensors **completely disconnected** from
  power.

Right before `esp_deep_sleep_start()`, the firmware drives `POWER_PIN` LOW,
cutting the sensors off entirely. The result is a **true deep sleep** where only
the sleeping MCU draws current — dramatically lower consumption than leaving the
sensors energized. On wake, the chip resets, `setup()` runs again, and the
sensors are re-powered.

> Wire the MOSFET to match your part. A logic-level **P-channel** high-side
> switch (or an N-channel low-side switch on the sensor ground, depending on
> your board) is typical. Make sure the gate logic matches the HIGH=on / LOW=off
> convention above.

---

## Hardware

| Component        | ESP32-C3 Pin         | Notes                              |
|------------------|----------------------|------------------------------------|
| DHT11 data       | `GPIO2`  (`DHTPIN`)  | Temperature + humidity             |
| Gas sensor (AO)  | `GPIO0`  (`GAS_SENSOR_PIN`) | Analog input (ADC)          |
| Sensor power gate| `GPIO10` (`POWER_PIN`)| Drives the power MOSFET           |
| USB / Serial     | —                    | 115200 baud for logs               |

Board: **`esp32-c3-devkitm-1`**.

---

## Project structure

```
Air-Sense/
├── platformio.ini            PlatformIO config + library deps
├── include/
│   ├── config.h              Pins & tuning constants (safe to commit)
│   ├── secrets.h             Real credentials (GITIGNORED)
│   ├── secrets.example.h     Template — copy to secrets.h
│   ├── sensor_data.h         Shared buffers + semaphore (declarations)
│   ├── sensors.h             Sensor API
│   └── comms.h               WiFi/Blynk + sleep API
└── src/
    ├── main.cpp              setup() / loop(), task creation
    ├── sensor_data.cpp       Shared buffers + semaphore (definitions)
    ├── sensors.cpp           Sensor init + acquisition task
    └── comms.cpp             WiFi/Blynk setup, upload task, deep sleep
```

### Module responsibilities

- **`config.h`** — all non-secret knobs: pins, sample count, thresholds, sleep
  duration, Blynk virtual pins.
- **`secrets.h`** — Blynk template/auth tokens and the WiFi AP password. Not in
  git; copy from `secrets.example.h`.
- **`sensor_data`** — the buffers shared between the two tasks and the binary
  semaphore that hands a finished batch from producer to consumer.
- **`sensors`** — powers the sensors, reads DHT + gas, fills the buffers, and
  signals when `SAMPLE_COUNT` readings are ready.
- **`comms`** — brings up WiFi (captive portal) and Blynk, averages each batch,
  uploads it, fires the gas alert, then powers down and deep sleeps.

---

## Runtime flow

```
setup()
  ├─ sensorsBegin()    power sensors on, init DHT
  ├─ commsBegin()      WiFiManager portal → WiFi → Blynk
  ├─ sensorDataInit()  create semaphore
  └─ start SensorTask (producer) + BlynkTask (consumer)

SensorTask  ── every 1s ──▶ read sensors ─▶ store sample
                                   │ (after 5 samples)
                                   ▼
                          give dataReadySemaphore
                                   │
BlynkTask   ── waits ──────────────┘
                          average → upload to Blynk
                          gas alert if mean ≥ threshold
                          POWER_PIN LOW (cut sensor power)
                          esp_deep_sleep_start (10 min)
                                   │
                          wake → chip resets → setup() again
```

---

## Setup & build

### Prerequisites
- [PlatformIO](https://platformio.org/) (CLI or the VS Code extension)

### 1. Add your credentials
```bash
cp include/secrets.example.h include/secrets.h
# edit include/secrets.h and fill in your Blynk + WiFi AP values
```

### 2. Build
```bash
pio run
```

### 3. Flash & monitor
```bash
pio run --target upload
pio device monitor
```

### 4. First-time WiFi provisioning
On first boot (or when no network is stored) the node opens a WiFi access point:

- **SSID:** `ESP Cyber` (configurable in `secrets.h`)
- **Password:** set in `secrets.h`

Connect to it, pick your network in the captive portal, and the node saves the
credentials for future boots.

---

## Configuration reference

All in `include/config.h`:

| Macro                  | Default        | Meaning                                   |
|------------------------|----------------|-------------------------------------------|
| `SAMPLE_COUNT`         | `5`            | Readings averaged per upload              |
| `GAS_OVERSAMPLE`       | `10`           | ADC reads averaged per gas sample         |
| `SENSOR_INTERVAL_MS`   | `1000`         | Delay between samples                     |
| `GAS_ALERT_THRESHOLD`  | `800`          | Mean gas value that triggers an alert     |
| `DEEP_SLEEP_US`        | `10 min`       | Sleep duration between batches            |
| `SLEEP_SETTLE_MS`      | `10000`        | Settle time after cutting sensor power    |
| `BLYNK_CONNECT_TIMEOUT_MS` | `10000`    | Blynk cloud connect timeout               |
| `WIFI_PORTAL_TIMEOUT_S`| `60`           | Captive-portal lifetime                   |

Blynk virtual pins: **V0** temperature, **V1** humidity, **V2** gas.

---

## Notes

- **Development fallback:** if the DHT11 returns `NaN` (disconnected/faulty), the
  firmware substitutes a random value so the dashboard keeps showing live data.
  Remove the `isnan(...)` fallbacks in `sensors.cpp` for production use.
- After deep sleep the ESP32 **resets** rather than resuming — execution always
  restarts from `setup()`.

---

## License

Educational / semester project. Use freely.
