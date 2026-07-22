#include "CronoScreen.h"
#include "config.h"
#include <Arduino.h>
#include <cstdio>

uint32_t CronoScreen::elapsedMs() const {
  return _running ? _accum + (millis() - _startMs) : _accum;
}

void CronoScreen::onButtonB() {
  if (_running) {                       // pausar
    _accum += millis() - _startMs;
    _running = false;
  } else {                              // iniciar / reanudar
    _startMs = millis();
    _running = true;
  }
}

void CronoScreen::onButtonBHold() {
  _running = false;
  _accum = 0;
}

void CronoScreen::draw(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  // Título + estado.
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString("CRONO", 6, 4);
  c.setTextDatum(top_right);
  c.setTextColor(_running ? cfg::COL_RING_STEP : cfg::COL_DIM);
  c.drawString(_running ? "|| pausar" : "> iniciar", W - 6, 4);

  // Tiempo MM:SS.d grande.
  uint32_t e = elapsedMs();
  uint32_t tenths = (e / 100) % 10;
  uint32_t secs   = (e / 1000) % 60;
  uint32_t mins   = e / 60000;
  char buf[12];
  snprintf(buf, sizeof(buf), "%02lu:%02lu.%lu",
           static_cast<unsigned long>(mins), static_cast<unsigned long>(secs),
           static_cast<unsigned long>(tenths));

  c.setFont(&fonts::Font4);
  c.setTextSize(2.0f);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(buf, W / 2, H / 2 + 4);
  c.setTextSize(1.0f);

  c.setFont(&fonts::Font2);
  c.setTextDatum(bottom_center);
  c.setTextColor(cfg::COL_DIM);
  c.drawString("mantener BtnB: cero", W / 2, H - 2);
}
