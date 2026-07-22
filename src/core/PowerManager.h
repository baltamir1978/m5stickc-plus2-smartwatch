#pragma once
#include <M5Unified.h>
#include <cstdint>

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

  int  batteryLevel();  // 0-100 (-1 si desconocido)
  bool isCharging();

private:
  bool     _screenOn = true;
  uint32_t _lastActivity = 0;
};
