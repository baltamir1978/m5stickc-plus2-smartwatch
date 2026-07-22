#include "core/PowerManager.h"
#include "config.h"
#include <Arduino.h>   // setCpuFrequencyMhz()

void PowerManager::begin() {
  _screenOn = true;
  _lastActivity = millis();
  _activeBrightness = cfg::BRIGHTNESS;
  setCpuFrequencyMhz(cfg::CPU_FREQ_ACTIVE);
  M5.Display.setBrightness(_activeBrightness);
}

void PowerManager::setBrightness(uint8_t v) {
  _activeBrightness = v;
  if (_screenOn) M5.Display.setBrightness(v);
}

void PowerManager::update() {
  if (_screenOn && (millis() - _lastActivity > _timeoutMs)) {
    sleepScreen();
  }
}

void PowerManager::notifyActivity() {
  _lastActivity = millis();
}

void PowerManager::wake() {
  if (_screenOn) return;
  _screenOn = true;
  setCpuFrequencyMhz(cfg::CPU_FREQ_ACTIVE);   // volver a plena velocidad
  M5.Display.wakeup();
  M5.Display.setBrightness(_activeBrightness);
  notifyActivity();
}

void PowerManager::sleepScreen() {
  if (!_screenOn) return;
  _screenOn = false;
  M5.Display.setBrightness(0);
  M5.Display.sleep();
  // Bajar la CPU ahorra energía; el BLE sigue vivo (modem sleep automático).
  // Nota: el envío IR y los tonos críticos ocurren con la pantalla encendida (240 MHz).
  setCpuFrequencyMhz(cfg::CPU_FREQ_IDLE);
}

int PowerManager::batteryLevel() {
  return M5.Power.getBatteryLevel();
}

bool PowerManager::isCharging() {
  return M5.Power.isCharging() == m5::Power_Class::is_charging_t::is_charging;
}
