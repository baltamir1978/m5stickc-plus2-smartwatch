#pragma once
#include "sensors/StepDetector.h"
#include "sensors/GestureDetector.h"

class FitnessTracker;

// Lee el acelerómetro del MPU6886 y alimenta al detector de pasos y al de gestos.
// Se muestrea desde el loop principal (~50 Hz), evitando accesos I2C concurrentes.
class MotionService {
public:
  void begin(FitnessTracker* fitness);
  void update();   // llamar en cada iteración del loop

  // Devuelve true una vez cuando se ha detectado un wrist-raise (para despertar pantalla).
  bool consumeWristRaise();

  // Si la pantalla está volteada, el gesto invierte el eje Z (orientación "mirando").
  void setFlipped(bool f) { _flip = f; }

private:
  FitnessTracker* _fitness = nullptr;
  StepDetector    _step;
  GestureDetector _gesture;
  bool            _enabled = false;
  bool            _wristRaise = false;
  bool            _flip = false;
};
