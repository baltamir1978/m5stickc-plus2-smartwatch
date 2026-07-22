#include "HistoryScreen.h"
#include "fitness/FitnessTracker.h"
#include "config.h"
#include <cstdio>
#include <algorithm>

void HistoryScreen::draw(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  FitnessTracker* f = _ctx->fitness;
  LocalTime t = _ctx->time->now();

  // 7 valores: 6 días completos anteriores + hoy (en vivo).
  uint32_t vals[7];
  for (int i = 0; i < 6; i++) vals[i] = f ? f->histDay(i) : 0;
  vals[6] = f ? f->steps() : 0;

  uint32_t scale = _ctx->stepGoal > 0 ? _ctx->stepGoal : 1;
  for (int i = 0; i < 7; i++) scale = std::max(scale, vals[i]);
  if (scale == 0) scale = 1;

  // Cabecera.
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString("PASOS 7 DIAS", 6, 4);
  c.setTextDatum(top_right);
  c.setTextColor(cfg::COL_CYAN);
  char today[10];
  snprintf(today, sizeof(today), "hoy %lu", static_cast<unsigned long>(vals[6]));
  c.drawString(today, W - 6, 4);

  // Área de la gráfica.
  const int baseY = H - 16;
  const int topY  = 26;
  const int maxBarH = baseY - topY;
  const int slot = (W - 12) / 7;
  const int barW = slot - 8;

  // Línea de objetivo.
  int goalY = baseY - static_cast<int>(maxBarH * (float)_ctx->stepGoal / scale);
  if (goalY > topY && goalY < baseY) {
    for (int x = 6; x < W - 6; x += 6) c.drawFastHLine(x, goalY, 3, cfg::COL_DIM);
  }

  // Barras + etiquetas de día.
  c.setFont(&fonts::Font2);
  for (int j = 0; j < 7; j++) {
    int x = 6 + j * slot + (slot - barW) / 2;
    int h = static_cast<int>(maxBarH * (float)vals[j] / scale);
    if (h < 1 && vals[j] > 0) h = 1;
    uint16_t col = (j == 6) ? cfg::COL_CYAN : cfg::COL_RING_STEP;
    if (h > 0) c.fillRect(x, baseY - h, barW, h, col);

    int wd = ((t.weekday - (6 - j)) % 7 + 7) % 7;
    c.setTextDatum(top_center);
    c.setTextColor((j == 6) ? cfg::COL_CYAN : cfg::COL_DIM);
    c.drawString(TimeService::weekdayShort(wd), x + barW / 2, baseY + 2);
  }
}
