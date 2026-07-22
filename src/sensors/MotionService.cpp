#include "sensors/MotionService.h"
#include "fitness/FitnessTracker.h"
#include <M5Unified.h>
#include <Arduino.h>
#include <cmath>

void MotionService::begin(FitnessTracker* fitness) {
  _fitness = fitness;
  _enabled = M5.Imu.isEnabled();
  _step.reset();
  _gesture.reset();
}

void MotionService::update() {
  if (!_enabled || _fitness == nullptr) return;

  M5.Imu.update();
  auto d = M5.Imu.getImuData();
  const float mag = sqrtf(d.accel.x * d.accel.x +
                          d.accel.y * d.accel.y +
                          d.accel.z * d.accel.z);
  const uint32_t nowMs = millis();

  // Pasos.
  int steps = _step.process(mag, nowMs);
  if (steps > 0) _fitness->addSteps(static_cast<uint32_t>(steps));

  // Gesto de levantar la muñeca.
  if (_gesture.update(d.accel.z, mag, nowMs)) _wristRaise = true;
}

bool MotionService::consumeWristRaise() {
  if (_wristRaise) {
    _wristRaise = false;
    return true;
  }
  return false;
}
