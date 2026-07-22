#include "EnvScreen.h"
#include "sensors/EnvSensor.h"
#include "config.h"
#include <cstdio>

void EnvScreen::draw(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  EnvSensor* e = _ctx->env;

  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString("ENV III", 6, 4);

  // Confirmación del sensor: dónde se detectó (Hat / Grove).
  c.setTextDatum(top_right);
  c.setTextColor(cfg::COL_DIM);
  c.drawString(e && e->present() ? e->source() : "--", W - 6, 4);

  char buf[16];
  c.setFont(&fonts::Font4);
  c.setTextDatum(middle_center);

  // Temperatura.
  snprintf(buf, sizeof(buf), "%.1f C", e ? e->temperature() : 0.0f);
  c.setTextColor(cfg::COL_ORANGE);
  c.drawString(buf, W / 2, 42);

  // Humedad.
  snprintf(buf, sizeof(buf), "%.0f %%", e ? e->humidity() : 0.0f);
  c.setTextColor(cfg::COL_CYAN);
  c.drawString(buf, W / 2, 74);

  // Presión.
  snprintf(buf, sizeof(buf), "%.0f hPa", e ? e->pressureHpa() : 0.0f);
  c.setTextColor(cfg::COL_RING_STEP);
  c.drawString(buf, W / 2, 106);
}
