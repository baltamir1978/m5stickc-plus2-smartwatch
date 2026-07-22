#include "WatchfaceScreen.h"
#include "fitness/FitnessTracker.h"
#include "config.h"
#include <cstdio>
#include <algorithm>

// ---------- Iconos dibujados con primitivas ----------
namespace {

uint16_t batteryColor(int level, bool charging) {
  if (charging) return cfg::COL_ACCENT;
  if (level >= 0 && level <= cfg::BATTERY_LOW_PCT) return cfg::COL_BAT_LOW;
  if (level >= 0 && level <= cfg::BATTERY_MID_PCT) return cfg::COL_ORANGE;
  return cfg::COL_ACCENT;
}

// Batería con relleno proporcional + terminal; rayo si está cargando.
void drawBattery(M5Canvas& c, int x, int y, int level, bool charging) {
  const int w = 24, h = 12;
  uint16_t col = batteryColor(level, charging);
  c.drawRoundRect(x, y, w, h, 2, col);
  c.fillRect(x + w, y + 3, 2, h - 6, col);         // terminal +
  int lv = level < 0 ? 0 : (level > 100 ? 100 : level);
  int fill = (w - 4) * lv / 100;
  if (fill > 0) c.fillRect(x + 2, y + 2, fill, h - 4, col);
  if (charging) {                                   // rayo (hueco sobre el relleno)
    int bx = x + w / 2;
    c.fillTriangle(bx + 1, y + 1, bx - 3, y + h / 2 + 1, bx + 1, y + h / 2, cfg::COL_BG);
    c.fillTriangle(bx - 1, y + h - 1, bx + 3, y + h / 2 - 1, bx - 1, y + h / 2, cfg::COL_BG);
  }
}

// Símbolo de Bluetooth (rune) con líneas.
void drawBluetooth(M5Canvas& c, int x, int y, uint16_t col) {
  const int H = 13;
  int cx = x + 4, top = y, bot = y + H, mid = y + H / 2;
  int q1 = y + H / 4, q3 = y + 3 * H / 4;
  int rx = x + 7, lx = x + 1;
  c.drawLine(cx, top, cx, bot, col);   // espina
  c.drawLine(cx, top, rx, q1, col);
  c.drawLine(rx, q1, lx, q3, col);
  c.drawLine(lx, q1, rx, q3, col);
  c.drawLine(rx, q3, cx, bot, col);
}

// Dos huellas de pie.
void drawFootsteps(M5Canvas& c, int x, int y, uint16_t col) {
  c.fillEllipse(x + 3, y + 4, 2, 4, col);
  c.fillCircle(x + 3, y + 9, 2, col);
  c.fillEllipse(x + 9, y + 6, 2, 4, col);
  c.fillCircle(x + 9, y + 11, 2, col);
}

// Icono de calendario simple.
void drawCalendar(M5Canvas& c, int x, int y, uint16_t col) {
  c.drawRoundRect(x, y + 1, 13, 12, 2, col);
  c.fillRect(x, y + 1, 13, 4, col);   // cabecera
  c.drawLine(x + 3, y, x + 3, y + 2, col);
  c.drawLine(x + 9, y, x + 9, y + 2, col);
}

}  // namespace

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

void WatchfaceScreen::drawClean(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();
  int  bat = _ctx->power->batteryLevel();
  bool chg = _ctx->power->isCharging();

  // --- Barra superior: fecha (izq, con icono) · BLE + batería (der, iconos) ---
  drawCalendar(c, 6, 6, cfg::COL_CYAN);
  char fecha[16];
  snprintf(fecha, sizeof(fecha), "%s %02d %s",
           TimeService::weekdayShort(t.weekday), t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_DATE);
  c.drawString(fecha, 24, 4);

  drawBattery(c, W - 28, 5, bat, chg);
  drawBluetooth(c, W - 44, 3, _ctx->bleConnected ? cfg::COL_CYAN : cfg::COL_DIM);

  // --- Hora HH:MM (grande, cian) ---
  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font7);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_CYAN);
  c.drawString(hora, W / 2, H / 2 - 2);

  // --- Pie: huellas + pasos + % + barra de progreso ---
  uint32_t steps = _ctx->fitness ? _ctx->fitness->steps() : 0;
  drawFootsteps(c, 6, H - 34, cfg::COL_ACCENT);
  char pasosStr[12];
  snprintf(pasosStr, sizeof(pasosStr), "%lu", static_cast<unsigned long>(steps));
  c.setFont(&fonts::Font2);
  c.setTextDatum(middle_left);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(pasosStr, 24, H - 24);

  int pct = (_ctx->stepGoal > 0)
                ? std::min(100, static_cast<int>(steps * 100 / _ctx->stepGoal)) : 0;
  char pctStr[6];
  snprintf(pctStr, sizeof(pctStr), "%d%%", pct);
  c.setTextDatum(middle_right);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString(pctStr, W - 6, H - 24);

  const int barX = 6, barY = H - 11, barW = W - 12, barH = 8;
  c.drawRoundRect(barX, barY, barW, barH, 3, cfg::COL_DIM);
  int fillW = (barW - 2) * pct / 100;
  if (fillW > 0) c.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, cfg::COL_ACCENT);
}

void WatchfaceScreen::drawMinimal(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();

  char fecha[10];
  snprintf(fecha, sizeof(fecha), "%02d %s", t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_right);
  c.setTextColor(cfg::COL_DIM);
  c.drawString(fecha, W - 4, 4);

  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font7);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(hora, W / 2, H / 2);

  int bat = _ctx->power->batteryLevel();
  drawBattery(c, W - 28, H - 16, bat, _ctx->power->isCharging());
}

void WatchfaceScreen::drawDashboard(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();

  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font4);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_CYAN);
  c.drawString(hora, 6, 6);

  char fecha[16];
  snprintf(fecha, sizeof(fecha), "%s %02d %s",
           TimeService::weekdayShort(t.weekday), t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_right);
  c.setTextColor(cfg::COL_DATE);
  c.drawString(fecha, W - 6, 10);

  c.drawFastHLine(6, 38, W - 12, cfg::COL_DIM);

  uint32_t steps = _ctx->fitness ? _ctx->fitness->steps() : 0;
  float    km    = _ctx->fitness ? _ctx->fitness->distanceMeters() / 1000.0f : 0.0f;
  int      kcal  = _ctx->fitness ? _ctx->fitness->calories() : 0;

  const int cols[3] = {W / 6, W / 2, 5 * W / 6};
  const char* labels[3] = {"PASOS", "KM", "KCAL"};
  const uint16_t vcol[3] = {cfg::COL_ACCENT, cfg::COL_CYAN, cfg::COL_ORANGE};
  char vals[3][12];
  snprintf(vals[0], sizeof(vals[0]), "%lu", static_cast<unsigned long>(steps));
  snprintf(vals[1], sizeof(vals[1]), "%.2f", km);
  snprintf(vals[2], sizeof(vals[2]), "%d", kcal);

  c.setTextDatum(middle_center);
  for (int i = 0; i < 3; i++) {
    c.setFont(&fonts::Font2);
    c.setTextColor(cfg::COL_DIM);
    c.drawString(labels[i], cols[i], 56);
    c.setTextColor(vcol[i]);
    c.drawString(vals[i], cols[i], 76);
  }

  int bat = _ctx->power->batteryLevel();
  drawBattery(c, W / 2 - 12, H - 16, bat, _ctx->power->isCharging());
  drawBluetooth(c, 8, H - 17, _ctx->bleConnected ? cfg::COL_CYAN : cfg::COL_DIM);
}
