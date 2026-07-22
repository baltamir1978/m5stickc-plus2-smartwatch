#pragma once
#include <M5Unified.h>
#include <vector>
#include "Screen.h"
#include "AppContext.h"

// Gestor de pantallas: mantiene la lista, redibuja una vez por minuto
// (o bajo demanda) y enruta los botones a la pantalla activa.
class UiManager {
public:
  void begin(AppContext* ctx);
  void update();        // redibuja si ha cambiado el minuto
  void forceRedraw();   // redibuja ya (p.ej. al despertar la pantalla)

  void nextScreen();
  void onButtonA();
  void onButtonB();
  void onButtonBHold();
  bool currentFullBright() const { return _screens[_current]->fullBrightness(); }

  // Overlay de llamada entrante, dibujado por encima de todo (parpadea con blink).
  void drawCallOverlay(const char* caller, bool blink);

  // Progreso del TV-B-Gone mientras se emiten los códigos IR.
  void drawIrProgress(const char* brand, int idx, int total);

  // Overlay de alarma (parpadea con blink).
  void drawAlarmOverlay(const char* timeStr, bool blink);

private:
  void render();

  AppContext*          _ctx = nullptr;
  M5Canvas             _canvas{&M5.Display};
  std::vector<Screen*> _screens;
  int                  _current = 0;
  int                  _lastMinute = -1;
  uint32_t             _lastSteps = 0;
  uint32_t             _lastTick = 0;
};
