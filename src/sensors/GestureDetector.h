#pragma once
#include <cstdint>

// Detección de "girar la muñeca para leer la hora".
//
// En vez de la orientación absoluta (que falla si ya partías con la pantalla hacia
// arriba, p.ej. tecleando), detecta el GIRO en sí: un pico de velocidad angular
// (giroscopio) que, al asentarse, deja la pantalla mirando al usuario (accelZ alto).
class GestureDetector {
public:
  void reset();

  // gyroMag = módulo de la velocidad angular (°/s); accelZ = componente Z del acelerómetro.
  // Devuelve true una vez cuando se detecta el gesto.
  bool update(float gyroMag, float accelZ, uint32_t nowMs);

private:
  bool     _turning = false;
  uint32_t _lastFire = 0;
};
