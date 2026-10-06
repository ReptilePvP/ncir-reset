# NCIR vertical lift - MEUS ME-X8 servo

## Current mode: full manual lift (reviewed 2026-10-06)

Full manual control is enabled (`motion_allowed = true`, `bench_test = false`).
Use the MEUS ME-X8 V1 positional servo identified by the supplied Amazon ASIN B0CB8N6KLC. Press in Lift to enable at center (25 mm),
then Up/Down changes the requested height across the 0-50 mm CAD range: light
deflection gives 1 mm steps and full deflection gives 5 mm steps.
The servo receives the destination directly, as it does when centering, and holds
the final command until released. There is no software speed ramp in normal mode.
Press again to release. The former 1.5-second test timeout does not apply.
The screen distinguishes moving from holding a commanded target; neither is
physical position feedback. Centering on enable remains an immediate command.

Motion remains disabled on boot; reconnecting after an I2C fault does not restart
it. Idle sleep stays inhibited while enabled or release is unconfirmed. Support
the carriage before release. Routine per-angle diagnostic readback now runs only
with Debug enabled (or optional bench mode); button/release events remain logged.

The user reported that the first servo test still blacked out the screen and a later test after reset did not. Successful full-range loaded movement
and power stability have not yet been established. Host regression tests cover
production full-range limits, holding beyond the old timeout, release, failures,
reconnect and rollover, plus the optional diagnostic mode.

ME-X8 build uploaded 2026-09-30: release build and host tests passed; ESP32-S3
detected on COM3, all four flash hashes verified, hard reset, PlatformIO SUCCESS.
Subsequent USB-reset logs reached interface rendered, Wi-Fi connected, NCIR init
OK and Boot ready. COM3 released. Boot still reports BUS_EN OFF and no detected
battery; no motor-load or black-screen resolution is claimed.

## Servo-specific profile (2026-09-30)

The supplied ASIN identifies MEUS ME-X8 (8.5 kg-cm class), not SG90. The
[manufacturer specification](https://www.meusracingb2b.com/products/brushless-high-torque-micro-servo)
lists nominal 180-degree travel over 500-2500 microseconds, 4.8-8.4 V operation,
and a 25T / 4.90 mm spline. This profile targets the V1 positional model;
it must not be assumed correct for the W360 continuous model or a different V2
travel specification. Check the actual label/version if it differs from the link.

The firmware now sends explicit 16-bit little-endian pulse commands to the U165
register `0x60 + 2 * channel`, and diagnostics read back that pulse register.
All angle-to-pulse commands are clamped to a hard minimum of 30 degrees (833 us)
to avoid driving below the lift's mechanical lower stop, including commands
outside the normal height mapping. The upper angle clamp is 180 degrees (2500 us);
normal height targets remain within 30-150 degrees. These are software command
limits; actual stop clearance must be checked on the assembled mechanism.
Center is 1500 us. Nominal 0/25/50 mm lift commands are 833/1500/2167 us, staying
inside the advertised full pulse range. The pulse-to-angle relationship is a
nominal mapping: actual travel, center, horn indexing and CAD fit still require
calibration. A stronger servo does not establish SG90 horn/cradle compatibility.

An external supply through U165 does not imply an 8.4 V servo output: this unit
provides a 5 V motor rail. Do not put 8.4 V on its 5 V terminal. Use its labeled
power inputs and manufacturer wiring. The code cannot remedy inadequate supply
current, voltage dips, mechanical binding or hardware damage. The first-enable
center step can still draw startup current; normal later targets are also direct commands.

## Previous brief unloaded servo test

The user replaced the modified servo with an unmodified positional servo and
requested a test before adding external power. That earlier configuration enabled
`bench_test`: press the joystick in Lift to command center, with automatic PWM
release after 1,500 ms on the next running control-loop tick. The test automatically
commands 90 -> 86 -> 94 -> 90 degrees, with one-degree slew steps. Up/down is ignored
in bench mode; full travel is disabled. Press again while active to release immediately. After
release, a new press is required for another test. No motion starts at boot.

Disconnect the horn from the mechanism and test the servo unloaded. Grove-only
power remains unverified and may still cause a reset. The software timeout cannot
guarantee release during power loss, a stalled processor or lost I2C connection.
If the screen goes black or the servo misbehaves, disconnect servo power and stop
the test. Passing this unloaded test does not qualify Grove power for the lift.

Host tests passed for the production bench-mode clamps, deadline, repeated-enable
deadline protection, rollover, and failed release, as well as the earlier motion
and fault handling tests. The older lockout history below is retained as evidence.

Bench build uploaded on 2026-09-20: all four flash hashes verified, hard reset,
PlatformIO SUCCESS. A subsequent USB-reset capture reached interface rendered,
Wi-Fi connected, NCIR init OK and Boot ready. COM3 released. The servo test itself
awaits the user's unloaded test; no motor command was initiated remotely.

Follow-up: user reported no movement from either servo while the UI showed test
active. Center-only commands can legitimately leave both a centered positional
servo and a continuous servo at neutral stationary. The revised test adds the
small automatic sweep above and serial readback of output mode and angle after
button actions and angle changes. Readback is controller register state, not
measured PWM, motor voltage or shaft position. CoreS3 `getExtOutput()` reports its
BUS_EN control bit; it is not a voltage measurement at the servo connector.

## Previous hardware issue and lockout

On 2026-09-20 the user confirmed the Tower Pro Micro Servo 9g was modified for
360-degree continuous rotation and the 8Servos unit was powered only by Grove.
Pressing Center caused continuous spinning and the CoreS3 screen went black.
Continuous-rotation conversion removes positional control: the existing crank
requires a positional servo with approximately 120 degrees of controlled travel.
That modified servo cannot provide the requested height or center without added
position feedback and a different controller design. Grove-only motor power is
also a suspected supply-drop cause; no voltage transient was measured.

The previous lockout build set `lift_config::motion_allowed = false`. Its Lift tab
displays the lockout, and pressing the joystick only retries PWM release. The
controller rejects enable requests without writing a servo angle or enabling
servo mode. Keep the servo disconnected until an unmodified positional servo
and appropriate motor supply are installed and checked. This lockout cannot
stop an independently powered controller if its I2C connection has failed.

The full-travel controls below describe intended positional-servo operation after
hardware checks. Full manual control is now enabled as described above.

The lockout build was uploaded to CoreS3 on COM3 on 2026-09-20. All four flash
regions passed hash verification and PlatformIO reported SUCCESS. A subsequent
USB reset reached `Boot: interface rendered`, Wi-Fi connected, `MLX90614 init OK`
and `Boot: ready`; COM3 was released. Host tests also verified that repeated
enable attempts in the production lockout configuration issue only release-mode
writes, with no servo-angle or servo-enable writes. Motor operation was disabled in that build; the electrical black-screen cause
was not measured.

The firmware follows `sg90_cores3_corrected/src/vertical.py` and `ANIMATION.md`
in the supplied M5Stack NCIR CAD directory. This revision is a **vertical-only,
50 mm guided lift**: the NCIR stays level and the CoreS3 stays fixed. The older
linked tilt design in `common.py` is not the exported assembly's motion.

## Wiring and configuration

- CoreS3 Port A -> Pa.Hub, address `0x70`, SDA GPIO2 / SCL GPIO1.
- Existing Joystick2 stays on Pa.Hub channel 1; NCIR stays on channel 5.
- Connect **Unit 8Servos U165**, default address `0x25`, to Pa.Hub **channel 0**.
- Connect the ME-X8 V1 positional servo to Unit 8Servos **output 0** (all channel numbers are zero-based).
  Match the servo's signal, 5 V and ground leads to the unit's printed pin labels.
- Use a suitable servo supply at the unit's documented power input. Do not apply
  its 9-24 V input voltage directly to the servo. Check the manufacturer's supply
  wiring and keep a common ground; do not assume the CoreS3 battery/Grove feed can
  sustain motor stall current.

`include/lift_control.h` contains the hub/output channels, address, center angle,
direction, permitted height and angle limits, and motion interval. The defaults assume the
stock horn is indexed horizontally at a 90-degree servo command. Increasing
servo angle is assumed to raise the lift; set `direction = -1` if needed.
The unused seven outputs are not configured by this firmware.

The driver uses the [official M5Stack U165 register interface](https://github.com/m5stack/M5Unit-8Servo/blob/main/src/M5_UNIT_8SERVO.h):
per-channel mode at `0x00 + channel` (3 = servo, 0 = input), pulse width at
`0x60 + 2 * channel` (two bytes, low byte first). Every transaction selects the appropriate Pa.Hub channel.
This is not the 8Servos HAT protocol. See the
[U165 documentation and supply schematic](https://docs.m5stack.com/en/unit/8Servos%20Unit).

## Controls

Navigate right past Cal to the **Lift** tab using Joystick2.
Hold Joystick2 with its connector on the right and stick on the left. Navigation
uses `x = 128 - rawY` and `y = rawX - 128` before low-pass filtering; positive
filtered Y raises the height target.

- **Press:** enable at the middle position (25 mm / default 90 degrees).
- **Up / Down:** light deflection adjusts by 1 mm; full deflection adjusts by 5 mm. Holding repeats at a 35 ms minimum interval, subject to loop timing.
- **Press while enabled:** release the PWM output. Support the carriage first.
- **Left / Right:** change tabs; the lift continues toward its target and holds.

The display shows the requested height and last acknowledged angle command.
These are **not measured position**: the servo provides no position feedback to this firmware.
Changing tabs does not release the load. Battery idle sleep is suppressed while
enabled or while release is unconfirmed; press to release before leaving it idle.
Power-off and firmware-triggered restart request release before proceeding.

At boot the firmware attempts to disable this channel's PWM when the unit is
present. It does not restore a saved height or automatically enable the servo.
The controller can remain powered independently of the CoreS3; firmware cannot
guarantee the output state before initialization or when the bus is disconnected.

An I2C failure cancels further motion and attempts release. If that fails, the
screen explicitly says **release UNCONFIRMED**: the motor may still be holding
its last command. Support the carriage and check power/cabling. After reconnect,
press once to confirm release, then press again to enable at center. There is no
automatic retry that starts movement. A missing optional unit does not prevent
temperature readings, fan controls or normal battery idle sleep at boot.

## Motion and first bench test

The CAD crank radius is `25 / sin(60 degrees)` mm. Its inverse kinematics are:

`crank_degrees = asin((height_mm - 25) / radius) * 180 / pi`

Servo command = center + direction * crank. Defaults map 0 / 25 / 50 mm to
30 / 90 / 150 degrees. Integer angle commands introduce up to approximately
0.26 mm of ideal geometric quantization; physical accuracy is unverified.
Normal commands send the requested destination directly at a 20 ms minimum write interval; optional bench mode retains one degree every 40 ms. Delayed loop iterations do not
catch up in a burst. **Initial enabling is an immediate center command** because
the current physical position is unknown; this first movement is not slew-limited.

1. With the horn disconnected from the loaded mechanism, power the controller,
   open Lift and enable center. Index the horn to the CAD horizontal middle pose.
2. Release and remove power before mechanically connecting the crank. Support
   the carriage so it cannot fall during assembly, release or loss of power.
3. For the first assembled run, narrow `min_height_mm` / `max_height_mm` around
   25 mm (for example 23 and 27), rebuild, and verify the direction with small steps.
4. Expand the permitted range only after checking guide binding, follower travel,
   cable slack and clearance. Verify low/middle/high against the printed model.
5. Verify release, reconnect behavior, temperature reads and both fan controls
   while moving. Verify battery idle behavior after release. Stop on stalls,
   buzzing against stops, excessive heat or CoreS3 resets.

## Software verification

Build firmware with `platformio run`. Host regression tests are in
`test/lift_control_test.cpp`; compile with a C++17 compiler and `-Iinclude`, then
run the resulting executable. They exercise the actual controller with a fake
I2C writer: CAD mapping, height and 30-180 degree pulse clamping, direct-command timing and optional bench slew, all enable-write failures, release,
disconnection, no automatic reconnect motion, and millis rollover.

These tests do not establish electrical communication, real servo direction,
loaded movement, physical fit or on-device screen layout. The new firmware must
be flashed and bench-tested to establish those results.

Current build and host-test evidence is recorded in [FLASH_AND_SMOKE_TEST.md](FLASH_AND_SMOKE_TEST.md). The dated upload records below describe earlier revisions.

Flashed to the connected CoreS3 on COM3 on 2026-09-20: esptool identified the
ESP32-S3, all four flash regions passed hash verification, the board was hard
reset, and PlatformIO reported SUCCESS. COM3 was checked available and released
after upload. Physical lift movement and on-device UI remain unverified.

Startup follow-up (2026-09-20): after a reported black screen with the hub
attached, disabled unused M5Unified external-display discovery, deferred external
power until after display initialization, added visible startup stages and a
50 ms Wire timeout, and rendered the UI before Wi-Fi connection. Rebuilt and
reflashed with all four region hashes verified. A USB-reset serial capture reached
`Boot: interface rendered`, Wi-Fi connected, `MLX90614 init OK`, and `Boot: ready`.
Visible-screen confirmation and a full power-cycle test with all peripherals
remain pending; the original fault mechanism is not proven. The same capture
reported external power OFF despite the enable request, and battery present=no;
do not treat this boot result as verification of servo power or battery health.

## Previous speed adjustment (2026-10-01)

The user reports the servo now responds with a 12 V / 2 A supply on HV, but moves
slowly. Normal command rate increased from 1 degree / 40 ms to 2 degrees / 20 ms.
Lift joystick repeat is now 1 mm / 70 ms rather than sharing the 220 ms menu
repeat. Other menus keep their existing repeat rate. Actual rates depend on the
main-loop cadence and mechanical load; these are command limits, not measured
shaft speed. No catch-up burst occurs after a delayed loop. Tests cover the new
rate, an ideal 1.2-second 120-degree traverse, odd-angle target clamping, faults,
rollover and the unchanged optional bench sequence.

### Current direct positioning follow-up

The user reports centering is fast but joystick movement remains slow. Removed
normal-mode incremental angle stepping so both paths send direct destinations.
Joystick repeat is now at least 35 ms, with 1 mm light-deflection steps and 5 mm
full-deflection steps. The same 0-50 mm clamps and I2C fault cancellation apply.
Bench diagnostics retain their slow sweep. Host tests verify direct endpoint
pulse writes, joystick step selection, limits, holding, faults and bench behavior.
Actual loaded movement speed remains a physical test, not a firmware timing claim.
