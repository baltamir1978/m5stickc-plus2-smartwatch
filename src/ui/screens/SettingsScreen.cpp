#include "SettingsScreen.h"
#include "core/Settings.h"
#include "config.h"
#include <cstdio>

void SettingsScreen::onButtonB() {
  Settings* s = _ctx->settings;
  if (!s) return;
  switch (_field) {
    case GOAL: {
      uint32_t g = s->stepGoal() + Settings::GOAL_STEP;
      if (g > Settings::GOAL_MAX) g = Settings::GOAL_MIN;
      s->setStepGoal(g);
      break;
    }
    case HEIGHT: {
      int h = s->height() + 1;
      if (h > Settings::HEIGHT_MAX) h = Settings::HEIGHT_MIN;
      s->setHeight(h);
      break;
    }
    case WEIGHT: {
      int w = s->weight() + 1;
      if (w > Settings::WEIGHT_MAX) w = Settings::WEIGHT_MIN;
      s->setWeight(w);
      break;
    }
  }
}

void SettingsScreen::onButtonBHold() {
  _field = (_field + 1) % FIELD_COUNT;
}

void SettingsScreen::draw(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  Settings* s = _ctx->settings;

  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString("AJUSTES", 6, 4);

  char rows[FIELD_COUNT][24];
  snprintf(rows[GOAL],   sizeof(rows[0]), "Objetivo  %lu",
           static_cast<unsigned long>(s ? s->stepGoal() : 0));
  snprintf(rows[HEIGHT], sizeof(rows[0]), "Altura    %d cm", s ? s->height() : 0);
  snprintf(rows[WEIGHT], sizeof(rows[0]), "Peso      %d kg", s ? s->weight() : 0);

  const int y0 = 30, dy = 24;
  for (int i = 0; i < FIELD_COUNT; i++) {
    bool sel = (i == _field);
    int y = y0 + i * dy;
    if (sel) {
      c.fillRoundRect(4, y - 2, W - 8, dy - 4, 3, cfg::COL_DIM);
      c.setTextColor(cfg::COL_TIME);
    } else {
      c.setTextColor(cfg::COL_DATE);
    }
    c.setTextDatum(top_left);
    c.drawString(rows[i], 10, y + 2);
  }

  c.setFont(&fonts::Font2);
  c.setTextColor(cfg::COL_DIM);
  c.setTextDatum(bottom_center);
  c.drawString("BtnB:+  manten:campo", W / 2, H - 2);
}
