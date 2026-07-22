#pragma once
#include <M5Unified.h>
#include <cstdint>
#include "config.h"

// Gestión de energía: brillo, apagado de pantalla por inactividad y batería.
// El M5StickC Plus 2 NO lleva AXP192; la batería se lee por ADC vía M5.Power.
class PowerManager {
public:
  void begin();

  // Llamar en cada loop(): apaga la pantalla si se supera INACTIVITY_MS.
  void update();

  // Reiniciar el temporizador de inactividad (p.ej. al pulsar un botón).
  void notifyActivity();

  void wake();          // enciende la pantalla
  void sleepScreen();   // apaga la pantalla (el firmware sigue activo)
  bool screenOn() const { return _screenOn; }

  void setInactivityMs(uint32_t ms) { _timeoutMs = ms; }   // tiempo de pantalla encendida
  void setBrightness(uint8_t v);                            // brillo activo (día/noche)

  int  batteryLevel();  // 0-100 (-1 si desconocido)
  bool isCharging();

private:
  bool     _screenOn = true;
  uint32_t _lastActivity = 0;
  uint32_t _timeoutMs = cfg::INACTIVITY_MS;
  uint8_t  _activeBrightness = cfg::BRIGHTNESS;
};
