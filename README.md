# Countdown Timer - Heltec Vision Master E213

A countdown display built on the **Heltec Vision Master E213** (ESP32-S3R8) development board. Shows the number of days remaining until a configurable target date on the 2.13" e-ink display, syncing time via NTP over WiFi and sleeping between updates to conserve power.

## Hardware

- [Heltec Vision Master E213](https://heltec.org/project/vision-master-e213/) (ESP32-S3, 2.13" e-ink, WiFi/BLE)
- USB-C cable

## Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or IDE plugin)
- Git

## Setup

### 1. Clone the repository

```bash
git clone <this-repo-url> countdown2
cd countdown2
```

### 2. Clone the heltec-eink-modules library

The display driver library must be cloned into the project root:

```bash
git clone git@github.com:todd-herbert/heltec-eink-modules.git
```

This places the library at `heltec-eink-modules/` alongside `src/`, which PlatformIO picks up via `lib_extra_dirs = .` in `platformio.ini`.

### 3. Configure

Copy the example config and edit it with your settings:

```bash
cp src/config.example.h src/config.h
```

- **WiFi** — set `WIFI_SSID` and `WIFI_PASSWORD`
- **Timezone** — set `GMT_OFFSET_SEC` (e.g. `-28800` for PST, `-18000` for EST)
- **Target date** — set `TARGET_YEAR`, `TARGET_MONTH`, `TARGET_DAY`
- **Update interval** — set `SLEEP_DURATION` in seconds (default: `3600` = 1 hour)

### 4. Build and upload

```bash
pio run -t upload
```

### 5. Monitor serial output

```bash
pio device monitor
```

## How it works

1. Wakes from deep sleep (or initial power-on)
2. Connects to WiFi and syncs time via NTP
3. Calculates days remaining until the target date
4. Renders the countdown on the e-ink display
5. Enters deep sleep for the configured duration
6. Repeats

## Project structure

```
countdown2/
├── src/
│   ├── main.cpp          # Application logic
│   └── config.h          # User configuration (WiFi, timezone, target date)
├── heltec-eink-modules/  # Display driver library (cloned separately)
├── platformio.ini        # PlatformIO build configuration
└── docs/
    └── PRD.md            # Product requirements document
```
