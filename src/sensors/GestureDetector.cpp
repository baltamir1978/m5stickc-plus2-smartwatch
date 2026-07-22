#include "sensors/GestureDetector.h"
#include "config.h"
#include <cmath>

void GestureDetector::reset() {
  _init = false;
  _viewing = false;
  _lastMotionMs = 0;
}

bool GestureDetector::update(float az, float mag, uint32_t nowMs) {
  if (!_init) {
    _gz = az;
    _init = true;
    return false;
  }

  // Orientación suavizada (gravedad en el eje normal a la pantalla).
  _gz += (az - _gz) * 0.2f;

  // ¿Hubo movimiento reciente? (|accel| se aparta de 1 g en reposo)
  if (fabsf(mag - 1.0f) > cfg::WRIST_MOTION_G) _lastMotionMs = nowMs;
  const bool recentMotion = (nowMs - _lastMotionMs) < cfg::WRIST_MOTION_WIN_MS;

  bool event = false;
  if (!_viewing && _gz > cfg::WRIST_VIEW_Z && recentMotion) {
    _viewing = true;
    event = true;
  } else if (_viewing && _gz < cfg::WRIST_DOWN_Z) {
    _viewing = false;  // brazo bajado: rearmar
  }
  return event;
}
