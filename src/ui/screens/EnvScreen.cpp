#include "EnvScreen.h"
#include "sensors/EnvSensor.h"
#include "config.h"
#include <cstdio>

// ---------- Iconos dibujados con primitivas ----------
namespace {

// Termómetro: tubo + bulbo.
void iconThermo(M5Canvas& c, int cx, int cy, uint16_t col) {
  c.fillRoundRect(cx - 3, cy - 11, 6, 15, 3, col);   // tubo
  c.fillCircle(cx, cy + 6, 6, col);                  // bulbo
}

// Gota de agua: punta + cuerpo.
void iconDrop(M5Canvas& c, int cx, int cy, uint16_t col) {
  c.fillTriangle(cx, cy - 11, cx - 6, cy + 1, cx + 6, cy + 1, col);
  c.fillCircle(cx, cy + 3, 6, col);
}

// Manómetro/barómetro: esfera con aguja.
void iconGauge(M5Canvas& c, int cx, int cy, uint16_t col) {
  c.drawCircle(cx, cy, 9, col);
  c.drawCircle(cx, cy, 8, col);          // aro más grueso
  c.drawLine(cx, cy, cx + 5, cy - 6, col);  // aguja
  c.fillCircle(cx, cy, 2, col);
}

}  // namespace

void EnvScreen::draw(M5Canvas& c) {
  const int W = c.width();
  c.fillScreen(cfg::COL_BG);

  EnvSensor* e = _ctx->env;

  // Cabecera: título + origen (Hat/Grove).
  c.setFont(&fonts::Font2);
  c.setTextColor(cfg::COL_ACCENT);
  c.setTextDatum(top_left);
  c.drawString("ENV III", 6, 4);
  c.setTextColor(cfg::COL_DIM);
  c.setTextDatum(top_right);
  c.drawString(e && e->present() ? e->source() : "--", W - 6, 4);

  // Tres filas centradas: icono + valor.
  const int iconX = 84;    // columna de iconos
  const int textX = 104;   // inicio de los valores
  const int rows[3] = {44, 76, 108};

  char buf[16];
  c.setTextDatum(middle_left);

  // Temperatura.
  iconThermo(c, iconX, rows[0], cfg::COL_ORANGE);
  snprintf(buf, sizeof(buf), "%.1f C", e ? e->temperature() : 0.0f);
  c.setFont(&fonts::Font4);
  c.setTextColor(cfg::COL_ORANGE);
  c.drawString(buf, textX, rows[0]);

  // Humedad.
  iconDrop(c, iconX, rows[1], cfg::COL_CYAN);
  snprintf(buf, sizeof(buf), "%.0f %%", e ? e->humidity() : 0.0f);
  c.setTextColor(cfg::COL_CYAN);
  c.drawString(buf, textX, rows[1]);

  // Presión.
  iconGauge(c, iconX, rows[2], cfg::COL_RING_STEP);
  snprintf(buf, sizeof(buf), "%.0f hPa", e ? e->pressureHpa() : 0.0f);
  c.setTextColor(cfg::COL_RING_STEP);
  c.drawString(buf, textX, rows[2]);
}
