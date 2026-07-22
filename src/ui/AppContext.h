#pragma once
#include <cstdint>
#include "config.h"
#include "../core/TimeService.h"
#include "../core/PowerManager.h"
#include "../core/Settings.h"
#include "../fitness/FitnessTracker.h"
#include "../ble/CallAlert.h"

// Estado compartido que las pantallas leen para dibujarse.
struct AppContext {
  TimeService*    time     = nullptr;
  PowerManager*   power    = nullptr;
  Settings*       settings = nullptr;
  FitnessTracker* fitness  = nullptr;
  CallAlert*      call     = nullptr;

  bool     bleConnected = false;
  uint32_t steps        = 0;                      // espejo de fitness->steps() para el watchface
  uint32_t stepGoal     = cfg::STEP_GOAL_DEFAULT;

  bool     tvbgoneRequested = false;              // lo activa UtilitiesScreen, lo ejecuta main
};
