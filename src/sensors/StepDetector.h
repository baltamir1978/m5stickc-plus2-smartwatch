#pragma once
#include <cstdint>

// Detector de pasos por picos sobre la magnitud del acelerómetro.
//
// Técnica: se suaviza la magnitud (|accel|) con un filtro exponencial, se
// mantiene una envolvente adaptativa (máx/mín con decaimiento) para fijar un
// umbral dinámico, y se cuenta un paso en cada flanco de subida que cruza el
// umbral con histéresis, siempre que la señal tenga suficiente amplitud y haya
// pasado un tiempo mínimo desde el último paso (periodo refractario).
class StepDetector {
public:
  // Procesa una muestra. mag = magnitud de la aceleración en g (~1.0 en reposo).
  // Devuelve el número de pasos detectados en esta muestra (0 ó 1).
  int process(float mag, uint32_t nowMs);

  void reset();

private:
  bool     _init = false;
  float    _filtered = 1.0f;
  float    _vmax = 1.1f;
  float    _vmin = 0.9f;
  bool     _above = false;
  uint32_t _lastStep = 0;

  static constexpr float    K          = 0.35f;  // suavizado (0..1)
  static constexpr float    DECAY      = 0.004f; // relajación de la envolvente por muestra
  static constexpr float    HYST       = 0.03f;  // histéresis alrededor del umbral (g)
  static constexpr float    MIN_SPREAD = 0.10f;  // amplitud mínima pico-valle (g)
  static constexpr uint32_t REFRACTORY = 260;    // ms mínimos entre pasos (~230 spm máx)
};
