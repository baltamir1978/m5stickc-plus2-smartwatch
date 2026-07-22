#include "WatchfaceScreen.h"
#include "fitness/FitnessTracker.h"
#include "config.h"
#include <cstdio>
#include <algorithm>

WatchfaceScreen::WatchfaceScreen(AppContext* ctx) : Screen(ctx) {
  _prefs.begin("watchface", false);
  _style = _prefs.getInt("style", CLEAN);
  if (_style < 0 || _style >= STYLE_COUNT) _style = CLEAN;
}

void WatchfaceScreen::onButtonB() {
  _style = (_style + 1) % STYLE_COUNT;
  _prefs.putInt("style", _style);
}

void WatchfaceScreen::draw(M5Canvas& c) {
  switch (_style) {
    case MINIMAL:   drawMinimal(c);   break;
    case DASHBOARD: drawDashboard(c); break;
    case CLEAN:
    default:        drawClean(c);     break;
  }
}

// Batería (con color de carga/baja) + estado BLE, arriba a la derecha.
void WatchfaceScreen::drawStatus(M5Canvas& c) {
  const int W = c.width();
  int bat = _ctx->power->batteryLevel();
  bool charging = _ctx->power->isCharging();

  uint16_t batColor = cfg::COL_DATE;
  if (charging)                                       batColor = cfg::COL_ACCENT;
  else if (bat >= 0 && bat <= cfg::BATTERY_LOW_PCT)   batColor = cfg::COL_BAT_LOW;

  char batStr[10];
  if (bat < 0) snprintf(batStr, sizeof(batStr), "--%%");
  else         snprintf(batStr, sizeof(batStr), "%s%d%%", charging ? "+" : "", bat);

  c.setFont(&fonts::Font2);
  c.setTextDatum(top_right);
  c.setTextColor(batColor);
  c.drawString(batStr, W - 6, 6);
  int batW = c.textWidth(batStr);

  c.setTextColor(_ctx->bleConnected ? cfg::COL_ACCENT : cfg::COL_DIM);
  c.drawString("BLE", W - 6 - batW - 8, 6);
}

void WatchfaceScreen::drawClean(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();

  char fecha[16];
  snprintf(fecha, sizeof(fecha), "%s %02d %s",
           TimeService::weekdayShort(t.weekday), t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_DATE);
  c.drawString(fecha, 6, 6);

  drawStatus(c);

  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font7);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(hora, W / 2, H / 2 - 4);

  uint32_t steps = _ctx->fitness ? _ctx->fitness->steps() : 0;
  c.setFont(&fonts::Font2);
  c.setTextColor(cfg::COL_DATE);
  char pasosStr[24];
  snprintf(pasosStr, sizeof(pasosStr), "%lu pasos", static_cast<unsigned long>(steps));
  c.setTextDatum(bottom_left);
  c.drawString(pasosStr, 8, H - 16);

  const int barX = 8, barY = H - 12, barW = W - 16, barH = 8;
  c.drawRoundRect(barX, barY, barW, barH, 3, cfg::COL_DIM);
  float frac = (_ctx->stepGoal > 0)
                   ? std::min(1.0f, static_cast<float>(steps) / _ctx->stepGoal) : 0.0f;
  int fillW = static_cast<int>((barW - 2) * frac);
  if (fillW > 0) c.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, cfg::COL_ACCENT);
}

void WatchfaceScreen::drawMinimal(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();

  // Fecha discreta arriba-derecha.
  char fecha[10];
  snprintf(fecha, sizeof(fecha), "%02d %s", t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_right);
  c.setTextColor(cfg::COL_DIM);
  c.drawString(fecha, W - 4, 4);

  // Hora enorme, centrada.
  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font7);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(hora, W / 2, H / 2);

  // Batería tenue abajo-derecha.
  int bat = _ctx->power->batteryLevel();
  char batStr[8];
  snprintf(batStr, sizeof(batStr), "%d%%", bat < 0 ? 0 : bat);
  c.setFont(&fonts::Font2);
  c.setTextDatum(bottom_right);
  c.setTextColor(cfg::COL_DIM);
  c.drawString(batStr, W - 4, H - 2);
}

void WatchfaceScreen::drawDashboard(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();

  // Hora (media) a la izquierda, fecha a la derecha.
  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font4);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(hora, 6, 6);

  char fecha[16];
  snprintf(fecha, sizeof(fecha), "%s %02d %s",
           TimeService::weekdayShort(t.weekday), t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_right);
  c.setTextColor(cfg::COL_DATE);
  c.drawString(fecha, W - 6, 10);

  c.drawFastHLine(6, 38, W - 12, cfg::COL_DIM);

  // Rejilla de métricas.
  uint32_t steps = _ctx->fitness ? _ctx->fitness->steps() : 0;
  float    km    = _ctx->fitness ? _ctx->fitness->distanceMeters() / 1000.0f : 0.0f;
  int      kcal  = _ctx->fitness ? _ctx->fitness->calories() : 0;

  const int cols[3] = {W / 6, W / 2, 5 * W / 6};
  const char* labels[3] = {"PASOS", "KM", "KCAL"};
  char vals[3][12];
  snprintf(vals[0], sizeof(vals[0]), "%lu", static_cast<unsigned long>(steps));
  snprintf(vals[1], sizeof(vals[1]), "%.2f", km);
  snprintf(vals[2], sizeof(vals[2]), "%d", kcal);

  c.setTextDatum(middle_center);
  for (int i = 0; i < 3; i++) {
    c.setFont(&fonts::Font2);
    c.setTextColor(cfg::COL_DIM);
    c.drawString(labels[i], cols[i], 56);
    c.setTextColor(cfg::COL_TIME);
    c.drawString(vals[i], cols[i], 76);
  }

  // Estado abajo.
  int bat = _ctx->power->batteryLevel();
  char st[24];
  snprintf(st, sizeof(st), "%s   BAT %d%%", _ctx->bleConnected ? "BLE ok" : "BLE --",
           bat < 0 ? 0 : bat);
  c.setFont(&fonts::Font2);
  c.setTextColor(cfg::COL_DATE);
  c.setTextDatum(bottom_center);
  c.drawString(st, W / 2, H - 4);
}
