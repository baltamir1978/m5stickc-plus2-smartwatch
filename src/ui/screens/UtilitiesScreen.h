#pragma once
#include "../Screen.h"

// Pantalla de utilidades. Por ahora: TV-B-Gone (BtnB lo dispara).
class UtilitiesScreen : public Screen {
public:
  explicit UtilitiesScreen(AppContext* ctx) : Screen(ctx) {}
  void draw(M5Canvas& canvas) override;
  void onButtonB() override;
};
