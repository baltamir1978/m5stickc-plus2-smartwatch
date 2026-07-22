#include "sensors/MotionService.h"
#include "fitness/FitnessTracker.h"
#include "config.h"
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
  const float gyroMag = sqrtf(d.gyro.x * d.gyro.x +
                              d.gyro.y * d.gyro.y +
                              d.gyro.z * d.gyro.z);
  const uint32_t nowMs = millis();

  // Log de calibración: acelerómetro + giroscopio una vez por segundo.
  static uint32_t lastDbg = 0;
  if (cfg::DEBUG_IMU && nowMs - lastDbg > 1000) {
    lastDbg = nowMs;
    Serial.printf("[IMU] az=%.2f |a|=%.2f gyro=%.0f dps\n", d.accel.z, mag, gyroMag);
  }

  // Pasos.
  int steps = _step.process(mag, nowMs);
  if (steps > 0) _fitness->addSteps(static_cast<uint32_t>(steps));

  // Gesto: girar la muñeca (giroscopio) terminando mirando la pantalla.
  // Si la pantalla está volteada, la orientación "mirando" tiene el eje Z invertido.
  const float zEff = _flip ? -d.accel.z : d.accel.z;
  if (_gesture.update(gyroMag, zEff, nowMs)) _wristRaise = true;
}

bool MotionService::consumeWristRaise() {
  if (_wristRaise) {
    _wristRaise = false;
    return true;
  }
  return false;
}
