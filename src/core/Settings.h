#pragma once
#include <Preferences.h>
#include <cstdint>

// Perfil de usuario editable en tiempo de ejecución, persistido en NVS.
// Lo usan FitnessTracker (zancada/calorías) y la UI (objetivo de pasos).
class Settings {
public:
  void begin();

  int      height()   const { return _height; }  // cm
  int      weight()   const { return _weight; }  // kg
  uint32_t stepGoal() const { return _goal; }

  void setHeight(int cm);
  void setWeight(int kg);
  void setStepGoal(uint32_t goal);

  // Rangos y pasos de edición.
  static constexpr int      HEIGHT_MIN = 120, HEIGHT_MAX = 220;
  static constexpr int      WEIGHT_MIN = 30,  WEIGHT_MAX = 200;
  static constexpr uint32_t GOAL_MIN = 1000, GOAL_MAX = 30000, GOAL_STEP = 500;

private:
  Preferences _prefs;
  int      _height = 181;
  int      _weight = 100;
  uint32_t _goal   = 10000;
};
