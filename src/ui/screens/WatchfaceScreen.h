#pragma once
#include <Preferences.h>
#include "../Screen.h"

// Watchfaces digitales conmutables con BtnB (elección persistida en NVS):
//   - CLEAN     : "Grande y limpio" (hora enorme + fecha + estado + pasos)
//   - MINIMAL   : solo la hora, enorme; fecha/batería discretas
//   - DASHBOARD : hora media + rejilla de métricas (pasos, km, kcal)
class WatchfaceScreen : public Screen {
public:
  explicit WatchfaceScreen(AppContext* ctx);
  void draw(M5Canvas& canvas) override;
  void onButtonB() override;   // cicla de esfera

private:
  enum Style { CLEAN = 0, MINIMAL, DASHBOARD, STYLE_COUNT };

  void drawClean(M5Canvas& c);
  void drawMinimal(M5Canvas& c);
  void drawDashboard(M5Canvas& c);
  int  dispHour(int h24) const;   // convierte a 12h si el ajuste lo pide

  Preferences _prefs;
  int         _style = CLEAN;
};
