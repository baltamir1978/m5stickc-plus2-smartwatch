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
    _steps = 0;
    _activeMinutes = 0;
    _stepsThisMinute = 0;
    _hourSteps = 0;
    _stoodMask = 0;
    _lastSavedSteps = 0;
    _inactivityFired = false;
    _dayKey = today;
    save(true);
  }

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

int FitnessTracker::calories() const {
  const float weight_kg = _settings ? _settings->weight() : cfg::USER_WEIGHT_KG;
  const float km = distanceMeters() / 1000.0f;
  return static_cast<int>(weight_kg * km * 0.9f + 0.5f);
}

bool FitnessTracker::consumeInactivityAlert() {
  if (_alertPending) {
    _alertPending = false;
    return true;
  }
  return false;
}

void FitnessTracker::save(bool force) {
  if (!force && _steps == _lastSavedSteps) return;  // nada que escribir
  _prefs.putInt("day", _dayKey);
  _prefs.putUInt("steps", _steps);
  _prefs.putUInt("active", _activeMinutes);
  _prefs.putUInt("stood", _stoodMask);
  _lastSavedSteps = _steps;
}
