#pragma once
#include <Preferences.h>
#include <cstdint>
#include "core/TimeService.h"

class Settings;

// Agrega la actividad del día a partir de los pasos detectados:
//   - distancia (zancada estimada por la altura),
//   - calorías (peso × distancia),
//   - minutos activos (minutos con cadencia por encima del umbral),
//   - aviso de inactividad en horario activo.
// Reinicia a medianoche y persiste el total del día en NVS (sobrevive a reinicios).
class FitnessTracker {
public:
  void begin(Settings* settings);

  // Llamado por MotionService cuando el detector cuenta pasos.
  void addSteps(uint32_t n);

  // Llamado en cada loop con la hora actual: gestiona cambios de minuto/día e inactividad.
  void update(const LocalTime& t);

  uint32_t steps() const          { return _steps; }
  float    distanceMeters() const;
  int      activeCalories() const;   // solo calorías por actividad (anillo Move)
  int      calories() const;         // total del día = basal (BMR prorrateado) + activas
  uint32_t activeMinutes() const  { return _activeMinutes; }
  int      standHours() const     { return __builtin_popcount(_stoodMask); }  // horas del día con actividad
  uint32_t histDay(int i) const   { return (i >= 0 && i < 6) ? _hist[i] : 0; } // 0=hace 6 días .. 5=ayer

  // Devuelve true una sola vez cuando se supera el umbral de inactividad.
  bool consumeInactivityAlert();

  // Objetivo recién alcanzado (una vez): 0 nada · 1 pasos · 2 calorías · 3 de pie.
  int consumeCelebration();

private:
  void save(bool force);
  static int ymd(const LocalTime& t) { return t.year * 10000 + t.month * 100 + t.day; }

  Settings*   _settings    = nullptr;
  Preferences _prefs;
  uint32_t _steps          = 0;
  uint32_t _activeMinutes  = 0;
  int      _dayKey         = -1;   // aaaammdd del día en curso
  int      _lastMinOfDay   = -1;
  uint32_t _stepsThisMinute = 0;
  uint32_t _secondsToday   = 0;   // segundos transcurridos del día (para BMR prorrateado)
  int      _curHour        = -1;   // hora en curso (para horas de pie)
  uint32_t _hourSteps      = 0;    // pasos en la hora en curso
  uint32_t _stoodMask      = 0;    // bitmask de horas del día "de pie"
  uint32_t _lastStepMs     = 0;
  uint32_t _lastSavedSteps = 0;
  uint32_t _hist[6]        = {0};   // pasos de los 6 días completos anteriores
  bool     _inactivityFired = false;
  bool     _alertPending    = false;
  bool     _fStep = false, _fCal = false, _fStand = false;  // objetivos ya celebrados hoy
  int      _celeb = 0;                                       // celebración pendiente
};
