#include "lift_control.h"
#include <assert.h>
#include <stdio.h>

static int calls = 0, fail_at = -1;
static bool disconnected = false;
static uint8_t last_reg, last_length;
static uint16_t last_value;
static bool writer(uint8_t reg, uint16_t value, uint8_t length) {
  last_reg = reg; last_value = value; last_length = length;
  return ++calls != fail_at && !disconnected;
}
int main() {
  assert(lift_config::joystickStepMm(50) == 1);
  assert(lift_config::joystickStepMm(-89) == 1);
  assert(lift_config::joystickStepMm(90) == 5);
  assert(lift_config::joystickStepMm(-128) == 5);
  LiftControl locked(writer, false); // Lockout remains independently testable.
  for (int press = 0; press < 3; ++press) {
    int before = calls;
    assert(!locked.enable(1000) && !locked.enabled);
    assert(calls == before + 1 && last_reg == 0 && last_value == 0);
    locked.target(50);
    locked.tick(2000);
    assert(calls == before + 1);
  }
  calls = 0;
  LiftControl lift(writer); // Actual production defaults: full manual travel.
  lift.tick(1000);
  assert(calls == 0 && !lift.enabled);
  assert(LiftControl::angleForHeight(-100) == 30);
  assert(LiftControl::angleForHeight(25) == 90);
  assert(LiftControl::angleForHeight(100) == 150);
  assert(LiftControl::pulseForAngle(0) == 500);
  assert(LiftControl::pulseForAngle(30) == 833);
  assert(LiftControl::pulseForAngle(90) == 1500);
  assert(LiftControl::pulseForAngle(150) == 2167);
  assert(LiftControl::pulseForAngle(180) == 2500);
  for (int mm = 0; mm <= 50; ++mm) {
    int angle = LiftControl::angleForHeight(mm);
    double height = 25 + 25 / sin(3.141592653589793 / 3) *
                             sin((angle - 90) * 3.141592653589793 / 180);
    assert(fabs(height - mm) < 0.26); // CAD inverse, including angle quantization.
  }
  assert(lift.enable(1000));
  assert(lift.target_mm == 25 && lift.commanded_deg == 90);
  assert(last_reg == 0x60 && last_length == 2 && last_value == 1500);
  lift.target(500);
  assert(lift.target_mm == 50);
  int before = calls;
  lift.tick(1019);
  assert(calls == before);
  lift.tick(1020);
  assert(lift.commanded_deg == 150 && last_reg == 0x60 && last_value == 2167 && last_length == 2);
  lift.tick(9000);
  assert(lift.commanded_deg == 150); // Holding does not alter the destination.
  for (uint32_t t = 9040; t <= 14000; t += 40) lift.tick(t);
  assert(lift.commanded_deg == 150);
  lift.tick(17000);
  assert(lift.enabled && lift.commanded_deg == 150); // Hold beyond the old bench timeout.
  lift.target(-100);
  lift.tick(17020);
  assert(lift.commanded_deg == 30 && last_value == 833); // Direct command, same path as center.
  for (uint32_t t = 17040; t <= 22000; t += 40) lift.tick(t);
  assert(lift.commanded_deg == 30 && lift.target_mm == 0);
  lift.target(50);
  for (uint32_t t = 22020; t <= 23200; t += 20) lift.tick(t);
  assert(lift.commanded_deg == 150);
  disconnected = true;
  lift.tick(23220);
  assert(!lift.enabled && lift.fault && lift.release_unconfirmed);
  disconnected = false;
  before = calls;
  lift.tick(30000);
  assert(calls == before); // Never automatically resumes after reconnect.
  assert(lift.release() && !lift.release_unconfirmed);
  for (int stage = 1; stage <= 4; ++stage) {
    fail_at = calls + stage;
    assert(!lift.enable(40000) && !lift.enabled && lift.fault);
  }
  fail_at = -1;
  assert(lift.enable(UINT32_MAX - 20));
  lift.target(26);
  lift.tick(20);
  assert(lift.commanded_deg == 92); // millis() rollover with the faster two-degree step.
  lift.target(37); // Maps to 115 degrees: odd delta must not overshoot.
  for (uint32_t t = 40; t <= 400; t += 20) lift.tick(t);
  assert(lift.commanded_deg == 115);
  assert(lift.release());
  before = calls;
  lift.target(50);
  lift.tick(100000);
  assert(!lift.enabled && calls == before && last_reg == 0 && last_value == 0);
  LiftControl bench(writer, true, true); // Retain regression coverage for optional bench mode.
  assert(bench.enable(1000));
  bench.target(100);
  assert(bench.target_mm == 25); // Manual commands cannot override the bench sequence.
  bench.target(-100);
  assert(bench.target_mm == 25);
  int lowest = 90, highest = 90;
  for (uint32_t t = 1040; t <= 2480; t += 40) {
    bench.tick(t);
    if (bench.commanded_deg < lowest) lowest = bench.commanded_deg;
    if (bench.commanded_deg > highest) highest = bench.commanded_deg;
  }
  assert(lowest == 86 && highest == 94 && bench.commanded_deg == 90);
  bench.tick(2499);
  assert(bench.enabled);
  bench.enable(2499);
  bench.tick(2500);
  assert(!bench.enabled && last_reg == 0 && last_value == 0);
  before = calls;
  bench.tick(9000);
  assert(calls == before);
  assert(bench.enable(UINT32_MAX - 500));
  bench.tick(999);
  assert(!bench.enabled); // Auto-release across millis rollover.
  assert(bench.enable(10000));
  disconnected = true;
  bench.tick(11500);
  assert(!bench.enabled && bench.release_unconfirmed);
  disconnected = false;
  puts("PASS: mapping, limits, slew, faults, lockout, bench deadline and rollover");
}
