#pragma once
#include <M5UnitENV.h>   // API simple: SHT3X (temp/humedad) + QMP6988 (presión)
#include <cstdint>

// Sensor ambiental ENV III conectado por el puerto Grove (I2C).
// Se detecta al arrancar; si no está, present() devuelve false y no se usa.
class EnvSensor {
public:
  void begin();                 // sondea el bus Grove
  bool present() const { return _present; }
  void update();                // lee los sensores (limitado a ~1 vez cada 2 s)

  float temperature() const { return _t; }  // °C
  float humidity()    const { return _h; }  // %
  float pressureHpa() const { return _p / 100.0f; }  // hPa (el sensor da Pa)
  const char* source() const { return _source; }    // dónde se detectó (Hat/Grove)

private:
  SHT3X    _sht;
  QMP6988  _qmp;
  bool     _present = false;
  const char* _source = "";
  float    _t = 0, _h = 0, _p = 0;
  uint32_t _lastRead = 0;
};
