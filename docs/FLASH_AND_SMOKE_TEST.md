# Flash and smoke-test checklist

Use this checklist for release builds of the NCIR Reset firmware. Do not record WiFi passwords or webhook tokens in test notes.

## Before flashing

1. Confirm `include/secrets.h` exists and contains non-placeholder values for `WIFI_SSID`, `WIFI_PASS`, and `FAN_WEBHOOK_URL`.
2. Confirm `include/secrets.h` is ignored:

   ```powershell
   git check-ignore -v include/secrets.h
   ```

3. Build the exact release image:

   ```powershell
   & "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run
   ```

4. Require a successful link and image generation. Record RAM and flash usage.

## Flash COM3

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --target upload --upload-port COM3
```

The flash passes only when esptool reports `Hash of data verified` for every segment, performs the hard reset, and PlatformIO exits with `SUCCESS`.

## On-device smoke test

### Boot and peripherals

- Display reaches the Live tab without a reset loop.
- NCIR reading updates and reacts when a warmer or cooler target is presented.
- Joystick changes tabs and its button performs the tab-specific action.
- Touch input is detected.
- Battery percentage is visible; `+` appears only while actively charging.

### USB power and sleep

1. Leave USB connected and provide no input for more than two minutes. The display must remain awake.
2. Disconnect USB and hold the measured scene steady. After two minutes without qualifying activity, the display should sleep.
3. Wake with touch and then with joystick input in separate trials.
4. Let the device sleep on battery, then connect USB. It should wake during the 200 ms sleep-poll cycle.
5. Confirm that a temperature change of at least 1 °F resets the idle timer.

### Fan webhook responsiveness

1. On the Live tab, press the joystick button.
2. `Sending...` should appear immediately and the UI should continue updating.
3. Additional presses during the request must not start duplicate requests.
4. A successful HTTP 2xx response should show `Fan ON` or `Fan OFF`.
5. Confirm the physical fan changed state; the displayed state is only a local record of successful toggles.
6. Repeat once with WiFi unavailable and confirm the UI remains usable through the bounded reconnect/HTTP path.

### Alerts and settings

- Enable an alert and cross the threshold; verify the two-tone alert and green highlight.
- Drop at least 5 °F below the threshold; verify hysteresis clears the alert.
- Change units, refresh interval, calibration, and debug; reboot and confirm persistence.
- Change emissivity only with a suitable target/reference, then verify the scheduled reboot completes.
- Use Settings → Power off and confirm the hardware power button restores operation.

## Release evidence

Record:

- Commit SHA
- Build date
- RAM and flash usage
- Firmware SHA-256
- Board/port used
- Pass/fail for each section above
- Any measured charge current, battery temperature, network latency, or fan mismatch
