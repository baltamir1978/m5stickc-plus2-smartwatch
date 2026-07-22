#pragma once
#include "../Screen.h"

// Pantalla de actividad: pasos grandes + distancia, calorías, minutos activos
// y barra de progreso hacia el objetivo diario.
class FitnessScreen : public Screen {
public:
  explicit FitnessScreen(AppContext* ctx) : Screen(ctx) {}
  void draw(M5Canvas& canvas) override;
};
