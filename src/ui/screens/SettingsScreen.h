#pragma once
#include "../Screen.h"

// Ajustes de perfil editables con un solo botón:
//   - BtnB clic     : sube el valor del campo seleccionado (envuelve al llegar al máximo)
//   - BtnB mantener : pasa al siguiente campo
//   - BtnA          : sale de la pantalla (ciclo global)
// Los cambios se guardan al momento en NVS (Settings).
class SettingsScreen : public Screen {
public:
  explicit SettingsScreen(AppContext* ctx) : Screen(ctx) {}
  void draw(M5Canvas& canvas) override;
  void onButtonB() override;
  void onButtonBHold() override;

private:
  enum Field { GOAL = 0, HEIGHT, WEIGHT, TIMEOUT, FLIP, DAY, NIGHT, BLE, FIELD_COUNT };
  int _field = GOAL;
};
