#include "sensors/StepDetector.h"
#include <algorithm>

void StepDetector::reset() {
  _init = false;
  _above = false;
  _lastStep = 0;
}

int StepDetector::process(float mag, uint32_t nowMs) {
  if (!_init) {
    _filtered = mag;
    _vmax = mag + 0.05f;
    _vmin = mag - 0.05f;
    _init = true;
    return 0;
  }

  // Suavizado exponencial.
  _filtered += (mag - _filtered) * K;

  // Envolvente adaptativa: sigue de inmediato hacia el extremo y se relaja poco a poco.
  _vmax = std::max(_filtered, _vmax - DECAY);
  _vmin = std::min(_filtered, _vmin + DECAY);

  const float thr    = (_vmax + _vmin) * 0.5f;
  const float spread = _vmax - _vmin;

  int steps = 0;
  if (!_above && _filtered > thr + HYST) {
    _above = true;
    if (spread > MIN_SPREAD && (nowMs - _lastStep) > REFRACTORY) {
      steps = 1;
      _lastStep = nowMs;
    }
  } else if (_above && _filtered < thr - HYST) {
    _above = false;
  }
  return steps;
}
