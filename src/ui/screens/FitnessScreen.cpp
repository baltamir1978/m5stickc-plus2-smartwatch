#include "FitnessScreen.h"
#include "fitness/FitnessTracker.h"
#include "../Icons.h"
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

  // --- Pasos en grande, centrado, con icono de huellas a la izquierda ---
  char stepStr[12];
  snprintf(stepStr, sizeof(stepStr), "%lu", static_cast<unsigned long>(steps));
  c.setFont(&fonts::Font7);
  int tw = c.textWidth(stepStr);
  int nx = W / 2;
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(stepStr, nx, 32);
  icons::steps(c, nx - tw / 2 - 12, 32, cfg::COL_RING_STEP);

  // --- Fila de métricas: icono (color del dato) + valor con unidad (fuente ampliada) ---
  const int cols[3] = {W / 6, W / 2, 5 * W / 6};
  const int icoY = 78, valY = 104;
  char km_s[12], kcal_s[12], min_s[12];
  snprintf(km_s,   sizeof(km_s),   "%.1f km", km);
  snprintf(kcal_s, sizeof(kcal_s), "%d kcal", kcal);
  snprintf(min_s,  sizeof(min_s),  "%lu min", static_cast<unsigned long>(amin));

  icons::pin(c,       cols[0], icoY, cfg::COL_CYAN);
  icons::flame(c,     cols[1], icoY, cfg::COL_RING_MOVE);
  icons::stopwatch(c, cols[2], icoY, cfg::COL_CYAN);

  c.setFont(&fonts::Font2);
  c.setTextSize(1.25f);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_CYAN);      c.drawString(km_s,   cols[0], valY);
  c.setTextColor(cfg::COL_RING_MOVE); c.drawString(kcal_s, cols[1], valY);
  c.setTextColor(cfg::COL_CYAN);      c.drawString(min_s,  cols[2], valY);
  c.setTextSize(1.0f);

  // --- Barra de progreso hacia el objetivo de pasos ---
  const int barX = 8, barY = H - 12, barW = W - 16, barH = 8;
  c.drawRoundRect(barX, barY, barW, barH, 3, cfg::COL_DIM);
  float frac = (_ctx->stepGoal > 0)
                   ? std::min(1.0f, static_cast<float>(steps) / _ctx->stepGoal) : 0.0f;
  int fillW = static_cast<int>((barW - 2) * frac);
  if (fillW > 0) c.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, cfg::COL_RING_STEP);
}
