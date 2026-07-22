#pragma once
#include "../Screen.h"
#include <cstdint>

// Cronómetro: BtnB clic = iniciar/pausar · BtnB mantener = poner a cero.
class CronoScreen : public Screen {
public:
  explicit CronoScreen(AppContext* ctx) : Screen(ctx) {}
  void draw(M5Canvas& canvas) override;
  void onButtonB() override;       // iniciar / pausar
  void onButtonBHold() override;   // reset
  bool wantsTick() const override { return _running; }

private:
  uint32_t elapsedMs() const;
  bool     _running = false;
  uint32_t _startMs = 0;
  uint32_t _accum   = 0;
};
