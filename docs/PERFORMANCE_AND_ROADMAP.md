# Performance and feature roadmap

This roadmap prioritizes changes by expected user-visible value. Measure before and after each optimization; current flash use is only about 22%, so responsiveness, I2C efficiency, and reliability matter more than binary size.

## Performance priorities

### P1 — Poll slow-changing power data less often

`getBatteryLevel()` and `isCharging()` currently run on every main-loop pass. Both can involve PMIC I2C work even though the UI only needs battery data every 1–5 seconds.

Recommended change:

- Add a `BATTERY_UPDATE_MS` scheduler (2 seconds is a good starting point).
- Cache battery percentage, charging state, battery voltage, and VBUS voltage.
- Use the cached VBUS state for ordinary sleep gating, with a direct read in the 200 ms sleep-wake loop.

Expected result: less internal I2C contention and more deterministic LVGL/sensor timing.

### P1 — Match sampling to useful sensor throughput

The selectable 35/60/100/150 ms intervals can request data faster than the MLX90614 is producing meaningful new measurements, depending on its configured refresh rate.

Recommended change:

- Confirm the sensor EEPROM refresh-rate setting.
- Default to the fastest interval that produces genuinely new values rather than repeated samples.
- Track sample age and I2C read failures; show stale data distinctly.
- Consider a small median or exponential filter for display stability while keeping raw values for threshold detection.

Expected result: lower bus load, steadier readings, and fewer false activity/alert transitions.

### P1 — Replace blocking WiFi reconnect with a state machine

The webhook is already off the UI task, but boot and wake reconnection can still wait several seconds.

Recommended change:

- Start WiFi asynchronously.
- Let the UI display `Connecting`, `OK`, or `Offline` while normal sensor work continues.
- Use exponential reconnect backoff with a manual retry action.

Expected result: instant screen availability after boot/wake and clearer network status.

### P2 — Use a FreeRTOS queue for webhook results

The worker task currently publishes small shared state directly. Move completion code, HTTP status, and notice text through a one-element queue consumed by the main task.

Expected result: single-threaded UI/state mutation and simpler concurrency reasoning.

### P2 — Profile the main loop

Add optional counters for:

- Loop time and worst-case loop time
- LVGL handler duration
- Sensor transaction duration/failure count
- Webhook reconnect and HTTP latency
- Free heap and minimum-ever free heap

Display these on a hidden diagnostics page or print them once every 10 seconds when debug mode is enabled.

### P3 — Trim LVGL configuration further only if needed

Logging, demos, examples, Lottie, QR, and vector/ThorVG are already disabled. Additional unused widgets, decoders, filesystems, themes, and fonts can be disabled later, but flash is not currently constrained.

## Reliability and safety priorities

1. **Authoritative fan state:** replace local toggle inference with separate ON/OFF commands or a status endpoint. A successful toggle request does not prove the final physical state.
2. **Fail-safe temperature control:** optionally turn the fan on automatically above a configurable threshold and keep manual override explicit. Define behavior for WiFi loss.
3. **Webhook security:** prefer HTTPS, avoid long-lived tokens in URLs when possible, and document certificate/time requirements.
4. **Watchdog and fault recovery:** count consecutive sensor/hub failures, reinitialize the affected bus/channel, and show a clear fault instead of silently retaining stale data.
5. **Battery protection validation:** measure charge current and battery temperature on the actual CoreS3/DIN base before treating 500 mA as hardware-validated.
6. **USB serial diagnostics:** enable the appropriate ESP32-S3 USB CDC build flags if reliable boot logs over the programming cable are desired.
7. **Automated checks:** add host-testable functions for thresholds, unit conversion, hysteresis, preference validation, and sleep decisions; add a compile workflow for pull requests.

## Recommended new features

### Highest value

- **Peak hold and session timer:** show peak temperature, time at/above target, and cooldown duration.
- **Temperature history graph:** a compact rolling 30–120 second chart with target and alert lines.
- **Emissivity presets:** common materials plus a custom value, with a warning that reflective metal needs special handling.
- **Fan automation mode:** Off / Manual / Auto with separate on/off thresholds and hysteresis.
- **Connection diagnostics:** WiFi RSSI, last HTTP code, last successful fan command, request latency, and retry action.

### Convenience

- **Captive-portal provisioning:** change WiFi and webhook settings without recompiling or storing credentials in source headers.
- **OTA firmware updates:** authenticated local-network updates with rollback or recovery instructions.
- **Profiles:** save named combinations of units, emissivity, calibration, target, and alert behavior.
- **Screen brightness and timeout controls:** user-selectable brightness and battery sleep duration.
- **Low-battery behavior:** warning, reduced brightness/sample rate, and clean shutdown threshold.

### Advanced

- **Logged sessions:** save timestamped CSV data to microSD for later analysis.
- **Home Assistant status integration:** publish temperature, battery, connectivity, and authoritative fan state through MQTT or a small REST endpoint.
- **Calibration wizard:** guided comparison against a trusted reference at multiple temperatures, with clear limits on accuracy.
- **Enclosure-aware mode:** detect or configure whether USB, DIN power, and the external sensor/head are installed and report wiring faults.
