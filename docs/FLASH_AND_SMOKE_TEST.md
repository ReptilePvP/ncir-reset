# Flash and smoke-test checklist

Use this checklist for release builds of the NCIR Reset firmware. Do not record WiFi passwords or webhook tokens in test notes.

## Before flashing

1. Confirm `include/secrets.h` exists and contains non-placeholder values for `WIFI_SSID`, `WIFI_PASS`, `FAN_WEBHOOK_URL`, and `SMOKE_FAN_WEBHOOK_URL`.
2. Confirm `include/secrets.h` is ignored:

   ```powershell
   git check-ignore -v include/secrets.h
   ```

3. Build the exact release image:

   ```powershell
   & "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e m5stack-cores3
   ```

4. Require a successful link and image generation. Record RAM and flash usage.

## Flash COM3

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e m5stack-cores3 --target upload --upload-port COM3
```

The flash passes only when esptool reports `Hash of data verified` for every segment, performs the hard reset, and PlatformIO exits with `SUCCESS`.

## On-device smoke test

### Boot and peripherals

- Display reaches the Live tab without a reset loop.
- NCIR reading updates and reacts when a warmer or cooler target is presented.
- Hold Joystick2 connector-right/stick-left; verify Left/Right changes tabs, Up/Down selects or adjusts the current control, and its button performs the tab-specific action.
- Touch input is detected.
- Battery percentage is visible; `+` appears only while actively charging.

### USB power and sleep

1. Leave USB connected and provide no input for more than two minutes. The display must remain awake.
2. Disconnect USB and hold the measured scene steady. After two minutes without qualifying activity, the display should sleep.
3. Wake with touch and then with joystick input in separate trials.
4. Let the device sleep on battery, then connect USB. It should wake during the 200 ms sleep-poll cycle.
5. Confirm that an object-temperature change of at least 1 F from the activity baseline resets the idle timer.

### Fan webhook responsiveness

1. On the Live tab, select Fan or Smoke Fan with Up/Down, then press the joystick button.
2. `Sending...` should appear immediately and the UI should continue updating.
3. Additional presses during the request must not start duplicate requests.
4. A successful HTTP 2xx response should show `Fan ON` or `Fan OFF`.
5. Confirm the physical fan changed state; the displayed state is only a local record of successful toggles.
6. Repeat for both controls, then once with WiFi unavailable and confirm the UI remains usable through the bounded reconnect/HTTP path.

### Alerts and settings

- Enable an alert and reach the threshold; verify the four-note ascending jingle and green highlight. Remaining above it must not repeat the jingle.
- Drop below the threshold; verify the highlight clears and the next threshold crossing plays the jingle again. Current alerts have no hysteresis or repeating cooldown.
- Change units, refresh interval, calibration, and debug; reboot and confirm persistence.
- Change emissivity only with a suitable target/reference, then verify the scheduled reboot completes.
- Use Settings -> Power off and confirm the hardware power button restores operation.

## Lift regression and guarded bench trial

Compile `test/lift_control_test.cpp` with a host C++17 compiler and `-Iinclude`, with assertions enabled, then run it. The fake writer checks inverse kinematics, pulse mapping including the 30-degree floor (833 us) and 180-degree ceiling (2500 us), direct targets, 20 ms scheduling, 1/5 mm joystick selection, holding beyond the former timeout, release, every enable-write failure, disconnect/reconnect, lockout, optional bench sweep and rollover. The host test does not exercise the application joystick axis transform.

For hardware checks, follow [NCIR_LIFT.md](NCIR_LIFT.md). Start unloaded with a verified ME-X8 V1 positional servo and appropriate U165 supply. Enable centers immediately. Support the carriage before release or disconnection. Narrow travel before the first assembled trial. Confirm direction, actual low/middle/high height, clearances, supply stability and UI responsiveness. Verify idle sleep remains inhibited while holding or release is unconfirmed, then resumes after release. Controller acknowledgement is not position feedback.

## Historical checkout evidence - 2026-10-06

- PlatformIO `m5stack-cores3` release build: SUCCESS.
- RAM: 141,748 / 327,680 bytes (43.3%).
- Flash: 1,424,981 / 6,553,600 bytes (21.7%).
- Host regression rebuilt from current source with the local Zig C++17 toolchain and passed, including the new pulse-floor and upper-clamp assertions. The installed standalone Clang could not build it because its host C headers were unavailable.
- Firmware binary SHA-256: `8a309154a6701db92c7b182a7607433f8646dc0e62bcde15d01ed5ddbeee9213`.
- No upload performed during this documentation update. Earlier COM3 uploads are historical records in the lift guide.
- Loaded lift travel, actual speed, supply transients, battery behavior, alert audio and physical webhook effects require observation.

## Release evidence

Record:

- Commit SHA
- Build date
- RAM and flash usage
- Firmware SHA-256
- Board/port used
- Pass/fail for each section above
- Any measured charge current, battery temperature, network latency, or fan mismatch

## UI revision evidence - 2026-10-07

- Cyberdeck-inspired NCIR//OS UI, temperature Lottie icons, and two distinct Lottie fan icons added.
- A device-observed `loopTask` stack overflow was corrected with a 32 KiB stack. The fan-animation build reached its 30-second checkpoint with 11,708 bytes minimum stack headroom and 87,468 bytes free heap; a 40-second serial observation showed no crash markers.
- The user confirmed COLD icon playback. GOOD/HOT transitions and fan ON/OFF visuals remain device checks.
- The latest direction/focus revision built and flashed to COM3: all four flash regions verified, hard reset completed, and COM3 released. Physical right/left direction and fan-focus locking remain to be confirmed by the user.
- On Live, Down selects Fan then Smoke Fan. Left/Right must not switch tabs while a fan is highlighted. Up past Fan clears the highlight and unlocks tabs. Press without a highlight must send no webhook.
- Fan ON plays; OFF freezes/dims; unknown remains dim with `--`. Verify these states after normal webhook use and confirm UI responsiveness with both fans ON. State is locally inferred, not authoritative smart-plug feedback.
