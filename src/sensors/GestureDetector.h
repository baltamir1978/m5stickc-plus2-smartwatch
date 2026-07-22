#pragma once
#include <cstdint>

// Detección de "wrist-raise": levantar la muñeca para mirar el reloj.
//
// Estrategia: se suaviza la componente del acelerómetro normal a la pantalla
// (gravedad en Z) para conocer la orientación, y se exige que el gesto venga
// acompañado de movimiento reciente (para no dispararlo al dejar el reloj boca
// arriba sobre una mesa). Emite el evento una sola vez al entrar en la zona de
// "mirando", y no vuelve a armarse hasta que el brazo baja.
class GestureDetector {
public:
  void reset();

  // Devuelve true una única vez cuando se detecta que se ha levantado la muñeca.
  bool update(float az, float mag, uint32_t nowMs);

private:
  bool     _init = false;
  float    _gz = 0.0f;           // gravedad suavizada en el eje normal a la pantalla
  bool     _viewing = false;
  uint32_t _lastMotionMs = 0;
};
