#include "FitnessScreen.h"
#include "fitness/FitnessTracker.h"
#include "config.h"
#include <cstdio>
#include <algorithm>

void FitnessScreen::draw(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  FitnessTracker* f = _ctx->fitness;
  uint32_t steps = f ? f->steps() : 0;
  float    km    = f ? f->distanceMeters() / 1000.0f : 0.0f;
  int      kcal  = f ? f->calories() : 0;
  uint32_t amin  = f ? f->activeMinutes() : 0;

  // Etiqueta.
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString("PASOS", 8, 6);

  // Número de pasos grande.
  char stepStr[12];
  snprintf(stepStr, sizeof(stepStr), "%lu", static_cast<unsigned long>(steps));
  c.setFont(&fonts::Font7);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(stepStr, W / 2, 52);

  // Fila de métricas.
  c.setFont(&fonts::Font2);
  c.setTextColor(cfg::COL_DATE);
  char l[16], m[16], r[16];
  snprintf(l, sizeof(l), "%.2f km", km);
  snprintf(m, sizeof(m), "%d kcal", kcal);
  snprintf(r, sizeof(r), "%lu min", static_cast<unsigned long>(amin));
  c.setTextDatum(middle_left);   c.drawString(l, 8, 92);
  c.setTextDatum(middle_center); c.drawString(m, W / 2, 92);
  c.setTextDatum(middle_right);  c.drawString(r, W - 8, 92);

  // Barra de progreso hacia el objetivo.
  const int barX = 8, barY = H - 14, barW = W - 16, barH = 10;
  c.drawRoundRect(barX, barY, barW, barH, 3, cfg::COL_DIM);
  float frac = (_ctx->stepGoal > 0)
                   ? std::min(1.0f, static_cast<float>(steps) / _ctx->stepGoal)
                   : 0.0f;
  int fillW = static_cast<int>((barW - 2) * frac);
  if (fillW > 0) {
    c.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, cfg::COL_ACCENT);
  }
}
