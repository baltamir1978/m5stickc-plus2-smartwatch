#include "sensors/GestureDetector.h"
#include "config.h"

void GestureDetector::reset() {
  _turning = false;
  _lastFire = 0;
}

bool GestureDetector::update(float gyroMag, float accelZ, uint32_t nowMs) {
  // Pico de giro por encima del umbral => estamos rotando la muñeca.
  if (gyroMag > cfg::WRIST_TURN_DPS) {
    _turning = true;
    return false;
  }

  // El giro se asienta (quieto de nuevo): si termina mirando la pantalla, disparar.
  if (_turning && gyroMag < cfg::WRIST_STILL_DPS) {
    _turning = false;
    if (accelZ > cfg::WRIST_MIN_Z && (nowMs - _lastFire) > cfg::WRIST_REFRACTORY_MS) {
      _lastFire = nowMs;
      return true;
    }
  }
  return false;
}
