#pragma once
#include "../Screen.h"

// Historial semanal de pasos: gráfica de barras de los últimos 7 días (hoy incluido).
class HistoryScreen : public Screen {
public:
  explicit HistoryScreen(AppContext* ctx) : Screen(ctx) {}
  void draw(M5Canvas& canvas) override;
};
