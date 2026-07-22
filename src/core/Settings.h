#pragma once
#include <Preferences.h>
#include <cstdint>

// Perfil de usuario editable en tiempo de ejecución, persistido en NVS.
// Lo usan FitnessTracker (zancada/calorías) y la UI (objetivo de pasos).
class Settings {
public:
  void begin();

  int      height()   const { return _height; }    // cm
  int      weight()   const { return _weight; }    // kg
  uint32_t stepGoal() const { return _goal; }
  uint8_t  rotation() const { return _rotation; }  // 1 o 3 (landscape)

  void setHeight(int cm);
  void setWeight(int kg);
  void setStepGoal(uint32_t goal);
  void toggleRotation();   // alterna 1 <-> 3 y persiste

  // Rangos y pasos de edición.
  static constexpr int      HEIGHT_MIN = 120, HEIGHT_MAX = 220;
  static constexpr int      WEIGHT_MIN = 30,  WEIGHT_MAX = 200;
  static constexpr uint32_t GOAL_MIN = 1000, GOAL_MAX = 30000, GOAL_STEP = 500;

private:
  Preferences _prefs;
  int      _height   = 181;
  int      _weight   = 100;
  uint32_t _goal     = 10000;
  uint8_t  _rotation = 3;
};
