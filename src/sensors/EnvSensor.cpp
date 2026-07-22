#include "sensors/EnvSensor.h"
#include <M5Unified.h>
#include <Arduino.h>
#include <Wire.h>

void EnvSensor::begin() {
  // El ENV III puede venir como HAT (conector inferior, G0/G26) o como Unit
  // (Grove, G32/G33). Probamos ambos, y ambos órdenes de SDA/SCL, y nos quedamos
  // con el par en el que responda el SHT30 (0x44).
  struct Pins { int sda, scl; const char* name; };
  static const Pins cands[] = {
    {0, 26, "HAT G0/G26"},
    {26, 0, "HAT G26/G0"},
    {32, 33, "Grove G32/G33"},
    {33, 32, "Grove G33/G32"},
  };

  for (const auto& p : cands) {
    Wire.end();
    Wire.begin(p.sda, p.scl);
    delay(5);
    Wire.beginTransmission(0x44);
    if (Wire.endTransmission() == 0) {
      _sht.begin(&Wire, 0x44, p.sda, p.scl, 400000);
      _qmp.begin(&Wire, 0x70, p.sda, p.scl, 400000);
      _present = true;
      _source = p.name;
      Serial.printf("[ENV] ENV III detectado en %s\n", p.name);
      return;
    }
  }

  _present = false;
  Serial.println("[ENV] ENV III no detectado.");
}

void EnvSensor::update() {
  if (!_present) return;
  if (millis() - _lastRead < 2000) return;
  _lastRead = millis();

  if (_sht.update()) {
    _t = _sht.cTemp;
    _h = _sht.humidity;
  }
  if (_qmp.update()) {
    _p = _qmp.pressure;   // Pa
  }
}
