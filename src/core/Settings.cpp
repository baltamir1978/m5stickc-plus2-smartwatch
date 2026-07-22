#include "core/Settings.h"
#include "config.h"

void Settings::begin() {
  _prefs.begin("profile", false);
  _height = _prefs.getInt("height", static_cast<int>(cfg::USER_HEIGHT_CM));
  _weight = _prefs.getInt("weight", static_cast<int>(cfg::USER_WEIGHT_KG));
  _goal   = _prefs.getUInt("goal", cfg::STEP_GOAL_DEFAULT);
}

void Settings::setHeight(int cm) {
  if (cm < HEIGHT_MIN) cm = HEIGHT_MIN;
  if (cm > HEIGHT_MAX) cm = HEIGHT_MAX;
  _height = cm;
  _prefs.putInt("height", _height);
}

void Settings::setWeight(int kg) {
  if (kg < WEIGHT_MIN) kg = WEIGHT_MIN;
  if (kg > WEIGHT_MAX) kg = WEIGHT_MAX;
  _weight = kg;
  _prefs.putInt("weight", _weight);
}

void Settings::setStepGoal(uint32_t goal) {
  if (goal < GOAL_MIN) goal = GOAL_MIN;
  if (goal > GOAL_MAX) goal = GOAL_MAX;
  _goal = goal;
  _prefs.putUInt("goal", _goal);
}
