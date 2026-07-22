#include "core/Settings.h"
#include "config.h"

void Settings::begin() {
  _prefs.begin("profile", false);
  _height = _prefs.getInt("height", static_cast<int>(cfg::USER_HEIGHT_CM));
  _weight = _prefs.getInt("weight", static_cast<int>(cfg::USER_WEIGHT_KG));
  _goal   = _prefs.getUInt("goal", cfg::STEP_GOAL_DEFAULT);
  _flip = _prefs.getBool("flip", false);
  _screenSecs = _prefs.getInt("scrsec", cfg::INACTIVITY_MS / 1000);
  _dayHour = _prefs.getInt("dayh", cfg::DAY_START_HOUR);
  _nightHour = _prefs.getInt("nighth", cfg::NIGHT_START_HOUR);
  _bleSaver = _prefs.getBool("blesv", false);
}

void Settings::toggleBleSaver() {
  _bleSaver = !_bleSaver;
  _prefs.putBool("blesv", _bleSaver);
}

void Settings::cycleDayHour() {
  _dayHour = (_dayHour + 1) % 24;
  _prefs.putInt("dayh", _dayHour);
}

void Settings::cycleNightHour() {
  _nightHour = (_nightHour + 1) % 24;
  _prefs.putInt("nighth", _nightHour);
}

void Settings::toggleFlip() {
  _flip = !_flip;
  _prefs.putBool("flip", _flip);
}

void Settings::cycleScreenTimeout() {
  static const int opts[] = {5, 10, 15, 30, 60};
  int i = 0;
  for (; i < 5; i++) if (opts[i] == _screenSecs) break;
  _screenSecs = opts[(i + 1) % 5];
  _prefs.putInt("scrsec", _screenSecs);
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
