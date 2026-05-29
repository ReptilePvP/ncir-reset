# NCIR Reset — M5Stack CoreS3 Temperature Monitor

Firmware for an **M5Stack CoreS3** that reads object temperature with an **MLX90614** (NCIR) sensor, shows a touch-friendly **LVGL** UI, controls a fan via **WiFi webhook**, and supports alerts, calibration, idle sleep, and power off.

Built with **PlatformIO**, **Arduino**, **M5Unified**, and **LVGL 9**.

---

## Table of contents

1. [Overview](#overview)
2. [Hardware](#hardware)
3. [Project structure](#project-structure)
4. [Getting started](#getting-started)
5. [Configuration](#configuration)
6. [User interface guide](#user-interface-guide)
7. [Joystick controls](#joystick-controls)
8. [Temperature zones](#temperature-zones)
9. [Power management](#power-management)
10. [Persisted settings](#persisted-settings)
11. [Reference — constants & defaults](#reference--constants--defaults)
12. [Reference — I2C map](#reference--i2c-map)
13. [Serial debug](#serial-debug)
14. [Libraries](#libraries)
15. [Troubleshooting](#troubleshooting)

---

## Overview

The device continuously measures **object temperature** (what the MLX90614 is pointed at). Readings are shown on a five-tab UI, color-coded by operating zone, and optionally compared against an alert threshold.

| Capability | Description |
|------------|-------------|
| Live display | Large temperature, bar graph, zone label, fan/WiFi/emissivity status |
| Statistics | Session min, max, last reading, read count |
| Settings | Units, refresh rate, emissivity, debug, power off |
| Alerts | Enable/disable and threshold with sound + green screen highlight |
| Calibration | Offset applied to object and ambient readings (stored in °F internally) |
| Fan control | HTTP POST to a webhook (e.g. Home Assistant) from the Live tab |
| Idle sleep | Display off after 2 minutes with no input; wake on joystick or touch |
| Power off | Full shutdown via PMIC (`M5.Power.powerOff()`); wake with hardware power button |

Ambient temperature is still read for the sensor but is **not** shown on the Live tab.

---

## Hardware

| Component | Role |
|-----------|------|
| **M5Stack CoreS3** | ESP32-S3, 320×240 display, touch, battery, speaker |
| **Pa.HUB** (I2C `0x70`) | Multiplexer on **Port A** |
| **Joystick2** (I2C `0x63`, hub **channel 1**) | Navigation and actions |
| **MLX90614** (I2C `0x5A`, hub **channel 5**) | Non-contact IR temperature |

**Port A I2C:** SDA = GPIO **2**, SCL = GPIO **1**, 100 kHz.

External **5V bus power** for Port A is enabled at boot (`M5.Power.setExtOutput(true)`).

```
CoreS3 (Port A)
    └── Pa.HUB (0x70)
            ├── Ch 1 → Joystick2 (0x63)
            └── Ch 5 → MLX90614 (0x5A)
```

---

## Project structure

```
Ncir Reset/
├── platformio.ini          # Board, libs, build flags
├── src/
│   └── main.cpp            # Application (UI, sensor, WiFi, power)
├── include/
│   ├── secrets.h.example   # Copy to secrets.h (gitignored)
│   ├── secrets.h           # Your WiFi + webhook (not in repo)
│   ├── lv_conf.h           # LVGL configuration
│   └── lvgl_m5gfx_compat.h # Display compatibility shim
└── README.md               # This file
```

---

## Getting started

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- USB cable to CoreS3
- Pa.HUB, Joystick2, and MLX90614 wired on Port A

### Secrets

```bash
cp include/secrets.h.example include/secrets.h
```

Edit `include/secrets.h`:

| Symbol | Purpose |
|--------|---------|
| `WIFI_SSID` | WiFi network name |
| `WIFI_PASS` | WiFi password |
| `FAN_WEBHOOK_URL` | URL for fan toggle (HTTP POST, empty JSON body `{}`) |

### Build and upload

```bash
pio run -e m5stack-cores3
pio run -e m5stack-cores3 -t upload
```

Serial monitor (115200 baud):

```bash
pio device monitor -e m5stack-cores3
```

---

## Configuration

All user-facing options are available on-device except WiFi and webhook URL (compile-time in `secrets.h`).

Changing **emissivity** in Settings writes the MLX90614 register and **restarts** the ESP32 after ~2.5 s so the sensor stack re-initializes cleanly.

---

## User interface guide

Five tabs at the top of the screen. **Battery** (`92%+`) is shown in the top-right on every tab.

### Live

| Element | Meaning |
|---------|---------|
| Hero card | Rounded panel with main reading |
| **425 F** | Object temperature (color = zone) |
| Bar | Level vs range (0–800 °F or 0–450 °C); fill color = zone |
| **COLD / GOOD / TOO HOT** | Zone label |
| **Fan** | Last known fan state after webhook (`ON` / `off` / `--`) |
| **WiFi** | `OK` or `--` |
| **e 0.95** | Current emissivity |
| Yellow notice | Fan webhook status (clears after 3 s) |
| Hint | `Press: toggle fan` |

Card border turns **green** when an alert is active.

### Stats

| Field | Description |
|-------|-------------|
| Min | Lowest object temp this session |
| Max | Highest object temp this session |
| Last | Most recent reading |
| Reads | Successful sensor read count |

### Settings

| Row | Press action |
|-----|----------------|
| Units | Toggle **F** / **C** |
| Refresh | Cycle sample interval: **35 / 60 / 100 / 150 ms** |
| Emissivity | Enter edit mode → Up/Down on slider → Press to save (restarts device) |
| Debug | Toggle serial debug logging |
| **Power off** | Save prefs and call `M5.Power.powerOff()` |

### Alerts

| Row | Press action |
|-----|----------------|
| Alerts | Toggle ON / OFF |
| Threshold | Edit mode → Up/Down → Press to save |

When enabled and object temp ≥ threshold: two-tone “target reached” sound, then repeating alert tones (1.5 s cooldown), green Live background until temp drops **5 °F** below threshold (hysteresis).

### Cal

| Row | Press action |
|-----|----------------|
| Offset | Edit mode → Up/Down (±5 °F steps, −150…+150) → Press to save |

Offset is added to both object and ambient raw readings before display.

---

## Joystick controls

Joystick is read over I2C (center ≈ **128**). Values are low-pass filtered.

| Input | Action (global) |
|-------|------------------|
| **Left / Right** | Previous / next tab |
| **Press** | Context action (see tab) |

| Tab | Up / Down | Press |
|-----|-----------|-------|
| **Live** | — | Toggle fan (webhook) |
| **Settings** | Move selection / adjust emissivity in edit mode | Change or apply |
| **Alerts** | Move selection / adjust threshold in edit mode | Change or apply |
| **Cal** | Move selection / adjust offset in edit mode | Save offset |
| **Stats** | — | — |

Repeat rate for held direction: **220 ms**.

---

## Temperature zones

Zones use **object temperature in °F** internally (even when displaying °C).

| Zone | Range (°F) | Color | Label |
|------|------------|-------|-------|
| Cold | &lt; 500 | Blue `#60A5FA` | COLD |
| Good | 500 – 610 | Green `#4ADE80` | GOOD |
| Too hot | &gt; 610 | Red `#F87171` | TOO HOT |

---

## Power management

### Idle sleep (2 minutes)

- Timer resets on: successful joystick read, tab change, or touch.
- After **120 s** idle (and not in an edit mode): display sleep, WiFi disconnect.
- Wake: joystick movement/button or screen touch → display on, WiFi reconnect.
- **3 s** cooldown after wake before sleep can trigger again.

Constants: `IDLE_SLEEP_TIMEOUT_MS`, `SLEEP_POLL_US` in `main.cpp`.

### Power off

Settings → **Power off** → press joystick.

Turn the unit back on with the **CoreS3 power button**. Preferences are saved before shutdown.

---

## Persisted settings

Stored in ESP32 **Preferences** namespace `uiflow`:

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `use_f` | bool | `true` | Fahrenheit vs Celsius |
| `refresh_idx` | int | `1` | Index into refresh table (60 ms) |
| `emissivity` | float | `0.95` | MLX90614 emissivity |
| `alerts_on` | bool | `false` | Alerts enabled |
| `alert_f` | float | `450` | Alert threshold (°F) |
| `cal_off_f` | float | `0` | Calibration offset (°F) |
| `debug_on` | bool | `false` | Serial debug |

---

## Reference — constants & defaults

### Timing

| Constant | Value | Purpose |
|----------|-------|---------|
| `LV_TICK_MS` | 5 | LVGL tick period |
| `UI_UPDATE_MS` | 80 | Screen refresh interval |
| `JOY_UPDATE_MS` | 35 | Joystick poll interval |
| `JOY_NAV_REPEAT_MS` | 220 | Key repeat while held |
| `JOY_BUTTON_DEBOUNCE_MS` | 220 | Button debounce |
| `IDLE_SLEEP_TIMEOUT_MS` | 120000 | Idle before sleep |
| `WIFI_CONNECT_TIMEOUT_MS` | 10000 | Initial WiFi connect |
| `HTTP_TIMEOUT_MS` | 5000 | Fan webhook timeout |
| `RESTART_DELAY_MS` | 2500 | Delay before reboot after emissivity save |

### Joystick

| Constant | Value |
|----------|-------|
| `JOY_CENTER` | 128 |
| `JOY_NAV_H_THRESH` / `JOY_NAV_V_THRESH` | 45 |
| `IDLE_JOY_ACTIVITY_THRESH` | 20 |

### Sliders

| Setting | Min | Max | Step |
|---------|-----|-----|------|
| Emissivity (×100) | 10 | 100 | 1 → 0.10–1.00 |
| Alert threshold (°F) | 100 | 800 | 5 |
| Calibration offset (°F) | −150 | 150 | 5 |

### Tab indices

| Index | Tab |
|-------|-----|
| 0 | Live |
| 1 | Stats |
| 2 | Settings |
| 3 | Alerts |
| 4 | Cal |

---

## Reference — I2C map

| Device | Address | Hub channel | Notes |
|--------|---------|-------------|-------|
| Pa.HUB | `0x70` | — | Channel select: `1 << ch` |
| Joystick2 | `0x63` | 1 | Reg `0x10` X, `0x11` Y, `0x20` button (0 = pressed) |
| MLX90614 | `0x5A` | 5 | Adafruit library |

---

## Serial debug

Enable **Debug: ON** in Settings. At 115200 baud you will see:

- Boot: Port A pins, battery, external power
- MLX init and emissivity
- Setting changes when adjusted
- Raw/adjusted temperatures (when debug on during reads)
- Fan webhook HTTP codes
- Sleep / wake messages

---

## Libraries

| Library | Version / source | Role |
|---------|------------------|------|
| [M5Unified](https://github.com/m5stack/M5Unified) | pinned commit | CoreS3 board, power, display, touch |
| [M5GFX](https://github.com/m5stack/M5GFX) | pinned commit | Display driver |
| [LVGL](https://lvgl.io/) | 9.3.0 | UI widgets |
| [Adafruit MLX90614](https://github.com/adafruit/Adafruit-MLX90614-Library) | ^2.1.5 | Temperature sensor |
| Adafruit BusIO | ^1.14.5 | I2C helper |

---

## Troubleshooting

| Symptom | Things to check |
|---------|-----------------|
| MLX init failed | Hub channel 5, wiring, sensor address `0x5A`, Port A power |
| WiFi `--` | `secrets.h`, signal, 10 s connect timeout; reconnect every 5 s cooldown |
| Fan webhook fails | URL in `secrets.h`, HTTP 2xx expected, serial debug for code |
| Sleep right after use | Ensure latest firmware (idle timer underflow fix) |
| Emissivity change needs reboot | By design; wait for automatic restart |
| UI clipped / overlapping | Live tab uses fixed layout; rebuild after UI changes |
| Power off won’t return | Normal — use hardware **power button** on CoreS3 |

---

## License & repository

Source: [ReptilePvP/ncir-reset](https://github.com/ReptilePvP/ncir-reset) on GitHub.

Do not commit `include/secrets.h` — it is listed in `.gitignore`.
