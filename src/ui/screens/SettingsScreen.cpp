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
    case TIMEOUT: s->cycleScreenTimeout(); break;
    case FLIP:
      s->toggleFlip();
      M5.Display.setRotation(s->rotation());   // reorientar la pantalla al instante
      break;
    case DAY:   s->cycleDayHour();   break;
    case NIGHT: s->cycleNightHour(); break;
    case BLE:   s->toggleBleSaver(); break;
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
  c.drawString("AJUSTES", 6, 3);

  char rows[FIELD_COUNT][24];
  snprintf(rows[GOAL],    sizeof(rows[0]), "Objetivo %lu",
           static_cast<unsigned long>(s ? s->stepGoal() : 0));
  snprintf(rows[HEIGHT],  sizeof(rows[0]), "Altura   %d cm", s ? s->height() : 0);
  snprintf(rows[WEIGHT],  sizeof(rows[0]), "Peso     %d kg", s ? s->weight() : 0);
  snprintf(rows[TIMEOUT], sizeof(rows[0]), "Apagar   %d s", s ? s->screenSecs() : 0);
  snprintf(rows[FLIP],    sizeof(rows[0]), "Pantalla %s",
           (s && s->flipped()) ? "volteada" : "normal");
  snprintf(rows[DAY],     sizeof(rows[0]), "Dia      %dh", s ? s->dayHour() : 0);
  snprintf(rows[NIGHT],   sizeof(rows[0]), "Noche    %dh", s ? s->nightHour() : 0);
  snprintf(rows[BLE],     sizeof(rows[0]), "BLE      %s",
           (s && s->bleSaver()) ? "ahorro" : "siempre");

  // Ventana de scroll: mantener el campo seleccionado visible.
  const int VIS = 5;                    // filas visibles
  const int listX = 4, listW = W - 12;  // deja hueco a la derecha para la barra
  const int y0 = 22, dy = 21;
  int start = 0;
  if (_field >= VIS) start = _field - VIS + 1;

  for (int i = 0; i < VIS && (start + i) < FIELD_COUNT; i++) {
    int f = start + i;
    int y = y0 + i * dy;
    if (f == _field) {
      c.fillRoundRect(listX, y - 2, listW, dy - 4, 3, cfg::COL_DIM);
      c.setTextColor(cfg::COL_TIME);
    } else {
      c.setTextColor(cfg::COL_DATE);
    }
    c.setTextDatum(top_left);
    c.drawString(rows[f], listX + 6, y + 2);
  }

  // Barra de desplazamiento (derecha): pista + pulgar proporcional a la posición.
  const int sbX = W - 5, sbY = y0 - 2, sbH = VIS * dy;
  c.drawFastVLine(sbX, sbY, sbH, cfg::COL_DIM);
  int thumbH = sbH * VIS / FIELD_COUNT;
  if (thumbH < 6) thumbH = 6;
  int maxStart = FIELD_COUNT - VIS;               // > 0 aquí
  int thumbY = sbY + (sbH - thumbH) * start / (maxStart > 0 ? maxStart : 1);
  c.fillRect(sbX - 1, thumbY, 3, thumbH, cfg::COL_ACCENT);
}
