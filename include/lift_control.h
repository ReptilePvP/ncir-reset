#pragma once
#include <math.h>
#include <stdint.h>

// U165 Unit 8Servos (not the 8Servos HAT). All channels are zero-based.
namespace lift_config {
// MEUS ME-X8 V1 positional servo: nominal 180 degrees over 500-2500 us.
constexpr int pulse_min_us = 500;
constexpr int pulse_max_us = 2500;
constexpr bool motion_allowed = true;
constexpr bool bench_test = false; // Opt-in diagnostic sweep; never used for normal lifting.
constexpr uint32_t test_duration_ms = 1500;
constexpr uint8_t hub_channel = 0;
constexpr uint8_t address = 0x25;
constexpr uint8_t servo_channel = 0;
// Index the horn at horizontal with a 90-degree command before assembly.
constexpr int center_deg = 90;
constexpr int direction = 1; // Set -1 if increasing angles lowers the carriage.
constexpr int min_height_mm = 0;
constexpr int max_height_mm = 50; // Reduce these limits for initial fit testing.
constexpr uint32_t step_ms = 20;
constexpr uint32_t joystick_repeat_ms = 35;
// Light deflection gives fine positioning; full deflection gives fast travel.
constexpr int joystick_fast_threshold = 90;
inline int joystickStepMm(int deflection) {
  return deflection >= joystick_fast_threshold || deflection <= -joystick_fast_threshold ? 5 : 1;
}
static_assert(hub_channel < 6 && servo_channel < 8, "Invalid lift channel");
static_assert(direction == 1 || direction == -1, "Invalid lift direction");
static_assert(center_deg >= 60 && center_deg <= 120, "Servo travel out of range");
static_assert(min_height_mm >= 0 && min_height_mm <= 25 &&
              max_height_mm >= 25 && max_height_mm <= 50, "Invalid lift limits");
}

// Hardware-independent controller; writer must select the Pa.Hub before each write.
class LiftControl {
 public:
  using Writer = bool (*)(uint8_t reg, uint16_t value, uint8_t length);
  explicit LiftControl(Writer writer, bool motion_allowed = lift_config::motion_allowed,
                       bool bench_test = lift_config::bench_test)
      : write_(writer), motion_allowed_(motion_allowed), bench_test_(bench_test) {}
  bool enabled = false;
  bool fault = false;
  bool release_unconfirmed = false;
  int target_mm = 25;
  int commanded_deg = lift_config::center_deg;

  static int angleForHeight(int mm) {
    mm = limit(mm, lift_config::min_height_mm, lift_config::max_height_mm);
    const double crank = asin((mm - 25.0) * sin(3.141592653589793 / 3.0) / 25.0)
                         * 180.0 / 3.141592653589793;
    return lift_config::center_deg + lift_config::direction * (int)lround(crank);
  }
  static uint16_t pulseForAngle(int angle) {
    angle = limit(angle, 0, 180);
    return lift_config::pulse_min_us +
        (angle * (lift_config::pulse_max_us - lift_config::pulse_min_us) + 90) / 180;
  }
  void target(int mm) {
    if (enabled && !bench_test_) target_mm = limit(mm, lift_config::min_height_mm,
                                                    lift_config::max_height_mm);
  }
  bool release() {
    enabled = false;
    // Input mode removes this channel's PWM; does not switch off the whole unit.
    release_unconfirmed = !write_(lift_config::servo_channel, 0, 1);
    fault = release_unconfirmed;
    return !fault;
  }
  bool enable(uint32_t now) {
    if (enabled) return true; // Repeated enable cannot extend a bench-test deadline.
    if (!release()) return false;
    if (!motion_allowed_) return false;
    target_mm = 25;
    commanded_deg = lift_config::center_deg;
    // Prepare center while PWM is disabled, then enable only this channel.
    if (!writeAngle(commanded_deg) ||
        !write_(lift_config::servo_channel, 3, 1) ||
        !writeAngle(commanded_deg)) {
      fail();
      return false;
    }
    enabled = true;
    fault = false;
    last_step_ = now;
    enabled_at_ = now;
    return true;
  }
  void tick(uint32_t now) {
    if (enabled && bench_test_ && (uint32_t)(now - enabled_at_) >= lift_config::test_duration_ms) {
      release();
      return;
    }
    const uint32_t interval = bench_test_ ? 40 : lift_config::step_ms;
    if (!enabled || (uint32_t)(now - last_step_) < interval) return;
    last_step_ = now;
    if (bench_test_) {
      const uint32_t elapsed = now - enabled_at_;
      // A visible unloaded check even if the servo starts at center.
      target_mm = elapsed < 250 ? 25 : elapsed < 650 ? 23 : elapsed < 1150 ? 27 : 25;
    }
    const int goal = angleForHeight(target_mm);
    // Also refresh while holding: detect a disconnected controller without moving.
    // Normal positioning uses the same direct pulse command as centering.
    // The optional unloaded diagnostic sweep alone retains slow stepping.
    const int next = bench_test_ ? commanded_deg + limit(goal - commanded_deg, -1, 1) : goal;
    if (!writeAngle(next)) { fail(); return; }
    commanded_deg = next;
  }
 private:
  Writer write_;
  const bool motion_allowed_;
  const bool bench_test_;
  uint32_t enabled_at_ = 0;
  uint32_t last_step_ = 0;
  bool writeAngle(int angle) {
    return write_(0x60 + 2 * lift_config::servo_channel, pulseForAngle(angle), 2);
  }
  static int limit(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
  void fail() {
    release();
    fault = true; // Reconnection never automatically resumes motion.
  }
};
