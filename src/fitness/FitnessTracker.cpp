#include "fitness/FitnessTracker.h"
#include "core/Settings.h"
#include "config.h"
#include <Arduino.h>  // millis()

void FitnessTracker::begin(Settings* settings) {
  _settings = settings;
  _prefs.begin("fitness", false);
  _dayKey        = _prefs.getInt("day", -1);
  _steps         = _prefs.getUInt("steps", 0);
  _activeMinutes = _prefs.getUInt("active", 0);
  _stoodMask     = _prefs.getUInt("stood", 0);
  _prefs.getBytes("hist", _hist, sizeof(_hist));   // histórico semanal (0 si no existe)
  _lastSavedSteps = _steps;
}

void FitnessTracker::addSteps(uint32_t n) {
  _steps          += n;
  _stepsThisMinute += n;
  _hourSteps      += n;
  _lastStepMs      = millis();
  _inactivityFired = false;  // al moverse, se rearma el aviso de inactividad

  // Marcar la hora en curso como "de pie" al superar el umbral de pasos.
  if (_curHour >= 0 && _hourSteps >= static_cast<uint32_t>(cfg::STAND_STEPS_HOUR)) {
    _stoodMask |= (1u << _curHour);
  }

  if (_steps - _lastSavedSteps >= 100) save(false);
}

void FitnessTracker::update(const LocalTime& t) {
  const int today = ymd(t);

  // Primer arranque del día: si el día guardado no coincide, reiniciar contadores.
  if (_dayKey < 0) {
    _dayKey = today;
  } else if (today != _dayKey) {
    // Guardar el día que termina en el histórico semanal (desplaza y añade).
    for (int i = 0; i < 5; i++) _hist[i] = _hist[i + 1];
    _hist[5] = _steps;
    _prefs.putBytes("hist", _hist, sizeof(_hist));

    _steps = 0;
    _activeMinutes = 0;
    _stepsThisMinute = 0;
    _hourSteps = 0;
    _stoodMask = 0;
    _lastSavedSteps = 0;
    _inactivityFired = false;
    _fStep = _fCal = _fStand = false;   // rearmar celebraciones
    _celeb = 0;
    _dayKey = today;
    save(true);
  }

  // Celebraciones de objetivos (una vez al día cada uno).
  if (!_fStep && _settings && _steps >= _settings->stepGoal()) { _fStep = true; _celeb = 1; }
  if (!_fCal && activeCalories() >= cfg::CALORIE_GOAL)         { _fCal = true;  _celeb = 2; }
  if (!_fStand && standHours() >= cfg::STAND_GOAL)            { _fStand = true; _celeb = 3; }

  _secondsToday = t.hour * 3600u + t.minute * 60u + t.second;

  // Cambio de hora: reiniciar el contador de pasos de la hora (para horas de pie).
  if (_curHour < 0) {
    _curHour = t.hour;
  } else if (t.hour != _curHour) {
    _curHour = t.hour;
    _hourSteps = 0;
  }

  // Cambio de minuto: consolidar el minuto anterior como activo o no.
  const int mod = t.hour * 60 + t.minute;
  if (_lastMinOfDay < 0) {
    _lastMinOfDay = mod;
  } else if (mod != _lastMinOfDay) {
    if (_stepsThisMinute >= cfg::ACTIVE_CADENCE_SPM) _activeMinutes++;
    _stepsThisMinute = 0;
    _lastMinOfDay = mod;
    save(false);  // persistencia periódica (una vez por minuto, si hubo cambios)
  }

  // Inactividad: en horario activo, si no hay pasos durante el umbral, avisar una vez.
  const bool activeHours = (t.hour >= cfg::ACTIVE_START_HOUR && t.hour < cfg::ACTIVE_END_HOUR);
  if (!_inactivityFired && activeHours && _lastStepMs != 0 &&
      (millis() - _lastStepMs > cfg::INACTIVITY_ALERT_MS)) {
    _inactivityFired = true;
    _alertPending = true;
  }
}

float FitnessTracker::distanceMeters() const {
  const float height_cm = _settings ? _settings->height() : cfg::USER_HEIGHT_CM;
  const float stride_m = height_cm * 0.415f / 100.0f;
  return _steps * stride_m;
}

int FitnessTracker::activeCalories() const {
  const float weight_kg = _settings ? _settings->weight() : cfg::USER_WEIGHT_KG;
  const float km = distanceMeters() / 1000.0f;
  return static_cast<int>(weight_kg * km * 0.9f + 0.5f);
}

int FitnessTracker::calories() const {
  // Metabolismo basal (Mifflin-St Jeor) prorrateado al tiempo transcurrido del día
  // + calorías por actividad.
  const float w = _settings ? _settings->weight() : cfg::USER_WEIGHT_KG;
  const float h = _settings ? _settings->height() : cfg::USER_HEIGHT_CM;
  const int   age  = _settings ? _settings->age() : 48;
  const bool  male = _settings ? (_settings->sex() == Settings::MALE) : true;

  const float bmr = 10.0f * w + 6.25f * h - 5.0f * age + (male ? 5.0f : -161.0f);
  const float basal = bmr * (_secondsToday / 86400.0f);
  return static_cast<int>(basal + activeCalories() + 0.5f);
}

bool FitnessTracker::consumeInactivityAlert() {
  if (_alertPending) {
    _alertPending = false;
    return true;
  }
  return false;
}

int FitnessTracker::consumeCelebration() {
  int c = _celeb;
  _celeb = 0;
  return c;   // 0 nada · 1 pasos · 2 calorías · 3 de pie
}

void FitnessTracker::save(bool force) {
  if (!force && _steps == _lastSavedSteps) return;  // nada que escribir
  _prefs.putInt("day", _dayKey);
  _prefs.putUInt("steps", _steps);
  _prefs.putUInt("active", _activeMinutes);
  _prefs.putUInt("stood", _stoodMask);
  _lastSavedSteps = _steps;
}
