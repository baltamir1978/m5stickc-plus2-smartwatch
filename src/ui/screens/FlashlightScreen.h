#pragma once
#include "../Screen.h"

// Linterna: pantalla en blanco a máximo brillo.
class FlashlightScreen : public Screen {
public:
  explicit FlashlightScreen(AppContext* ctx) : Screen(ctx) {}
  void draw(M5Canvas& canvas) override;
  bool fullBrightness() const override { return true; }
};
