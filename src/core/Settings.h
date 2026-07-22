#pragma once
#include <Preferences.h>
#include <cstdint>

// Perfil de usuario editable en tiempo de ejecución, persistido en NVS.
// Lo usan FitnessTracker (zancada/calorías) y la UI (objetivo de pasos).
class Settings {
public:
  void begin();

  enum Sex { MALE = 0, FEMALE = 1 };

  int      height()   const { return _height; }    // cm
  int      weight()   const { return _weight; }    // kg
  int      sex()      const { return _sex; }       // MALE / FEMALE
  int      age()      const { return _age; }        // años
  uint32_t stepGoal() const { return _goal; }
  bool     flipped()  const { return _flip; }
  uint8_t  rotation() const { return _flip ? 1 : 3; }      // 3 = normal, 1 = volteada
  int      screenSecs() const { return _screenSecs; }      // segundos de pantalla encendida
  uint32_t screenTimeoutMs() const { return static_cast<uint32_t>(_screenSecs) * 1000; }
  int      dayHour()   const { return _dayHour; }          // hora de inicio del brillo de día
  int      nightHour() const { return _nightHour; }        // hora de inicio del brillo de noche
  bool     bleSaver()  const { return _bleSaver; }         // BLE apagado salvo sync diario
  bool     alarmOn()   const { return _alarmOn; }
  int      alarmHour() const { return _alarmHour; }
  int      alarmMin()  const { return _alarmMin; }
  bool     hour24()    const { return _hour24; }

  void setHeight(int cm);
  void setWeight(int kg);
  void setStepGoal(uint32_t goal);
  void toggleSex();             // hombre <-> mujer y persiste
  void cycleAge();              // +1 año (AGE_MIN..AGE_MAX) y persiste
  void toggleFlip();            // gira la pantalla 180° y persiste
  void cycleScreenTimeout();    // 5 -> 10 -> 15 -> 30 -> 60 -> 5 s y persiste
  void cycleDayHour();          // +1 h (0..23) y persiste
  void cycleNightHour();        // +1 h (0..23) y persiste
  void toggleBleSaver();        // BLE ahorro <-> siempre y persiste
  void toggleAlarm();           // alarma on/off y persiste
  void cycleAlarmHour();        // +1 h (0..23) y persiste
  void cycleAlarmMin();         // +5 min (0..55) y persiste
  void toggleHour24();          // 24h <-> 12h y persiste

  // Rangos y pasos de edición.
  static constexpr int      HEIGHT_MIN = 120, HEIGHT_MAX = 220;
  static constexpr int      WEIGHT_MIN = 30,  WEIGHT_MAX = 200;
  static constexpr int      AGE_MIN = 5, AGE_MAX = 99;
  static constexpr uint32_t GOAL_MIN = 1000, GOAL_MAX = 30000, GOAL_STEP = 500;

private:
  Preferences _prefs;
  int      _height     = 181;
  int      _weight     = 100;
  int      _sex        = MALE;
  int      _age        = 48;
  uint32_t _goal       = 10000;
  bool     _flip       = false;
  int      _screenSecs = 5;
  int      _dayHour    = 8;
  int      _nightHour  = 21;
  bool     _bleSaver   = false;
  bool     _alarmOn    = false;
  int      _alarmHour  = 7;
  int      _alarmMin   = 0;
  bool     _hour24     = true;
};
