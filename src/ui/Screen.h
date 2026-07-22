#pragma once
#include <M5Unified.h>
#include "AppContext.h"

// Interfaz base de una pantalla. Cada pantalla dibuja sobre el canvas (sprite).
class Screen {
public:
  explicit Screen(AppContext* ctx) : _ctx(ctx) {}
  virtual ~Screen() = default;

  virtual void onEnter() {}
  virtual void draw(M5Canvas& canvas) = 0;
  virtual void onButtonA() {}      // BtnA: ciclar / acción primaria
  virtual void onButtonB() {}      // BtnB (clic): acción secundaria
  virtual void onButtonBHold() {}  // BtnB (mantener): acción terciaria

protected:
  AppContext* _ctx;
};
