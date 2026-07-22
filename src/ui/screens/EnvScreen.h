#pragma once
#include "../Screen.h"

// Pantalla del sensor ambiental ENV III (temp / humedad / presión).
// Solo se añade al ciclo de pantallas si el sensor está conectado.
class EnvScreen : public Screen {
public:
  explicit EnvScreen(AppContext* ctx) : Screen(ctx) {}
  void draw(M5Canvas& canvas) override;
  void onEnter() override;
  void onExit() override;
};
