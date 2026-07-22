#pragma once
#include <cstdint>

// Detector de pasos con filtro de cadencia.
//
// 1) Detecta "candidatos": picos en la magnitud del acelerómetro (umbral dinámico
//    con histéresis y amplitud mínima).
// 2) Solo cuenta cuando los candidatos llegan en RITMO de caminar (intervalos entre
//    MIN_INTERVAL y MAX_INTERVAL). Tras WARMUP candidatos rítmicos seguidos confirma
//    la marcha y acredita también los de calentamiento; si se rompe el ritmo, reinicia.
//    Esto descarta golpes sueltos/irregulares (teclear, gestos) que no son marcha.
class StepDetector {
public:
  // Procesa una muestra (mag = |accel| en g). Devuelve pasos acreditados (0, 1 o WARMUP).
  int process(float mag, uint32_t nowMs);
  void reset();

private:
  bool     _init = false;
  float    _filtered = 1.0f;
  float    _vmax = 1.1f;
  float    _vmin = 0.9f;
  bool     _above = false;
  uint32_t _lastCand = 0;   // instante del último candidato
  int      _run = 0;        // candidatos rítmicos consecutivos

  static constexpr float    K            = 0.25f;  // suavizado (menor = más filtrado)
  static constexpr float    DECAY        = 0.004f; // relajación de la envolvente por muestra
  static constexpr float    HYST         = 0.06f;  // histéresis alrededor del umbral (g)
  static constexpr float    MIN_SPREAD   = 0.13f;  // amplitud mínima pico-valle (g) — bajar = capta pasos flojos
  static constexpr uint32_t MIN_INTERVAL = 250;    // ms mínimos entre pasos (más rápido = ruido)
  static constexpr uint32_t MAX_INTERVAL = 1300;   // ms máximos en ritmo (más lento = rompe marcha)
  static constexpr int      WARMUP       = 2;      // candidatos rítmicos para confirmar marcha
};
