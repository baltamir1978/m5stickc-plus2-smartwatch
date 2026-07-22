#include "core/Settings.h"
#include "config.h"

void Settings::begin() {
  _prefs.begin("profile", false);
  _height = _prefs.getInt("height", static_cast<int>(cfg::USER_HEIGHT_CM));
  _weight = _prefs.getInt("weight", static_cast<int>(cfg::USER_WEIGHT_KG));
  _sex    = _prefs.getInt("sex", MALE);
  _age    = _prefs.getInt("age", 48);
  _goal   = _prefs.getUInt("goal", cfg::STEP_GOAL_DEFAULT);
  _flip = _prefs.getBool("flip", false);
  _screenSecs = _prefs.getInt("scrsec", cfg::INACTIVITY_MS / 1000);
  _dayHour = _prefs.getInt("dayh", cfg::DAY_START_HOUR);
  _nightHour = _prefs.getInt("nighth", cfg::NIGHT_START_HOUR);
  _bleSaver = _prefs.getBool("blesv", false);
  _alarmOn   = _prefs.getBool("almon", false);
  _alarmHour = _prefs.getInt("almh", 7);
  _alarmMin  = _prefs.getInt("almm", 0);
  _hour24    = _prefs.getBool("h24", true);
}

void Settings::toggleHour24() {
  _hour24 = !_hour24;
  _prefs.putBool("h24", _hour24);
}

void Settings::toggleAlarm() {
  _alarmOn = !_alarmOn;
  _prefs.putBool("almon", _alarmOn);
}

void Settings::cycleAlarmHour() {
  _alarmHour = (_alarmHour + 1) % 24;
  _prefs.putInt("almh", _alarmHour);
}

void Settings::cycleAlarmMin() {
  _alarmMin = (_alarmMin + 5) % 60;
  _prefs.putInt("almm", _alarmMin);
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

void Settings::toggleSex() {
  _sex = (_sex == MALE) ? FEMALE : MALE;
  _prefs.putInt("sex", _sex);
}

void Settings::cycleAge() {
  _age = (_age >= AGE_MAX) ? AGE_MIN : _age + 1;
  _prefs.putInt("age", _age);
}

void Settings::setStepGoal(uint32_t goal) {
  if (goal < GOAL_MIN) goal = GOAL_MIN;
  if (goal > GOAL_MAX) goal = GOAL_MAX;
  _goal = goal;
  _prefs.putUInt("goal", _goal);
}
