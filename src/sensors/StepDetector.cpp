#include "sensors/StepDetector.h"
#include <algorithm>

void StepDetector::reset() {
  _init = false;
  _above = false;
  _lastCand = 0;
  _run = 0;
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

  // Envolvente adaptativa: sigue al extremo y se relaja poco a poco.
  _vmax = std::max(_filtered, _vmax - DECAY);
  _vmin = std::min(_filtered, _vmin + DECAY);

  const float thr    = (_vmax + _vmin) * 0.5f;
  const float spread = _vmax - _vmin;

  // Detección de candidato: flanco de subida sobre el umbral con amplitud suficiente.
  bool candidate = false;
  if (!_above && _filtered > thr + HYST) {
    _above = true;
    if (spread > MIN_SPREAD) candidate = true;
  } else if (_above && _filtered < thr - HYST) {
    _above = false;
  }
  if (!candidate) return 0;

  // Filtro de cadencia sobre los candidatos.
  const uint32_t iv = nowMs - _lastCand;
  _lastCand = nowMs;

  if (iv < MIN_INTERVAL) return 0;              // demasiado rápido: ruido
  if (iv > MAX_INTERVAL) { _run = 1; return 0; } // se rompió el ritmo: reiniciar marcha

  _run++;
  if (_run == WARMUP) return WARMUP;            // marcha confirmada: acreditar el calentamiento
  if (_run > WARMUP)   return 1;                // marcha sostenida
  return 0;                                     // aún calentando
}
