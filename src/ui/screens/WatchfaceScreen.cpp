#include "WatchfaceScreen.h"
#include "fitness/FitnessTracker.h"
#include "config.h"
#include <cstdio>
#include <algorithm>

// ---------- Iconos y anillos dibujados con primitivas ----------
namespace {

uint16_t batteryColor(int level, bool charging) {
  if (charging) return cfg::COL_ACCENT;
  if (level >= 0 && level <= cfg::BATTERY_LOW_PCT) return cfg::COL_BAT_LOW;
  if (level >= 0 && level <= cfg::BATTERY_MID_PCT) return cfg::COL_ORANGE;
  return cfg::COL_ACCENT;
}

// Batería con relleno proporcional + terminal; rayo amarillo si está cargando.
void drawBattery(M5Canvas& c, int x, int y, int level, bool charging) {
  const int w = 24, h = 12;
  uint16_t col = batteryColor(level, charging);
  c.drawRoundRect(x, y, w, h, 2, col);
  c.fillRect(x + w, y + 3, 2, h - 6, col);         // terminal +
  int lv = level < 0 ? 0 : (level > 100 ? 100 : level);
  int fill = (w - 4) * lv / 100;
  if (fill > 0) c.fillRect(x + 2, y + 2, fill, h - 4, col);
  if (charging) {                                   // rayo amarillo, bien visible
    int bx = x + w / 2, by = y + 1, bh = h - 2, mid = by + bh / 2;
    c.fillTriangle(bx + 2, by,      bx - 3, mid,     bx + 1, mid,      cfg::COL_BOLT);
    c.fillTriangle(bx - 2, by + bh, bx + 3, mid,     bx - 1, mid,      cfg::COL_BOLT);
  }
}

// Símbolo de Bluetooth (rune) con líneas.
void drawBluetooth(M5Canvas& c, int x, int y, uint16_t col) {
  const int H = 13;
  int cx = x + 4, top = y, bot = y + H, mid = y + H / 2;
  int q1 = y + H / 4, q3 = y + 3 * H / 4;
  int rx = x + 7, lx = x + 1;
  c.drawLine(cx, top, cx, bot, col);
  c.drawLine(cx, top, rx, q1, col);
  c.drawLine(rx, q1, lx, q3, col);
  c.drawLine(lx, q1, rx, q3, col);
  c.drawLine(rx, q3, cx, bot, col);
}

// Un anillo: pista tenue (círculo completo) + arco de progreso proporcional.
void drawRing(M5Canvas& c, int cx, int cy, int r0, int r1, float frac, uint16_t col) {
  c.fillArc(cx, cy, r0, r1, 0, 360, cfg::COL_DIM);
  if (frac > 0.0f) {
    if (frac > 1.0f) frac = 1.0f;
    c.fillArc(cx, cy, r0, r1, 0, static_cast<int>(360.0f * frac), col);
  }
}

// 3 anillos concéntricos estilo Apple: calorías (fuera), pasos (medio), de pie (dentro).
void drawRings(M5Canvas& c, int cx, int cy, float calF, float stepF, float standF) {
  drawRing(c, cx, cy, 27, 33, calF,   cfg::COL_RING_MOVE);
  drawRing(c, cx, cy, 19, 25, stepF,  cfg::COL_RING_STEP);
  drawRing(c, cx, cy, 11, 17, standF, cfg::COL_RING_STAND);
}

// --- Iconos de métricas (centrados en cx,cy, ~16px) ---

void icoSteps(M5Canvas& c, int cx, int cy, uint16_t col) {   // huellas
  c.fillEllipse(cx - 3, cy - 2, 2, 4, col);
  c.fillCircle(cx - 3, cy + 3, 2, col);
  c.fillEllipse(cx + 3, cy, 2, 4, col);
  c.fillCircle(cx + 3, cy + 5, 2, col);
}

void icoPin(M5Canvas& c, int cx, int cy, uint16_t col) {     // marcador (distancia)
  c.fillCircle(cx, cy - 3, 6, col);
  c.fillTriangle(cx - 5, cy, cx + 5, cy, cx, cy + 8, col);
  c.fillCircle(cx, cy - 3, 2, cfg::COL_BG);                  // agujero
}

void icoFlame(M5Canvas& c, int cx, int cy, uint16_t col) {   // llama (calorías)
  c.fillTriangle(cx, cy - 9, cx - 5, cy + 3, cx + 5, cy + 3, col);
  c.fillCircle(cx, cy + 3, 5, col);
  c.fillCircle(cx, cy + 4, 2, cfg::COL_BG);                  // hueco interior
}

void icoStopwatch(M5Canvas& c, int cx, int cy, uint16_t col) {  // cronómetro (activo)
  c.drawCircle(cx, cy + 1, 7, col);
  c.drawCircle(cx, cy + 1, 6, col);
  c.fillRect(cx - 2, cy - 9, 4, 3, col);                     // botón
  c.drawLine(cx, cy + 1, cx + 3, cy - 3, col);               // aguja
}

void icoPerson(M5Canvas& c, int cx, int cy, uint16_t col) {  // persona (de pie)
  c.fillCircle(cx, cy - 6, 3, col);                          // cabeza
  c.fillRoundRect(cx - 2, cy - 2, 4, 9, 2, col);             // cuerpo
  c.drawFastHLine(cx - 4, cy + 8, 9, col);                   // suelo
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

  // --- Fecha (grande, arriba-izquierda) ---
  char fecha[16];
  snprintf(fecha, sizeof(fecha), "%s %02d %s",
           TimeService::weekdayShort(t.weekday), t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font4);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_DATE);
  c.drawString(fecha, 6, 2);

  // --- Batería (con rayo) + BLE, arriba-derecha ---
  drawBattery(c, W - 28, 4, bat, chg);
  drawBluetooth(c, W - 44, 3, _ctx->bleConnected ? cfg::COL_CYAN : cfg::COL_DIM);

  // --- Hora HH:MM (izquierda, cian) ---
  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font7);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_CYAN);
  c.drawString(hora, 78, 62);

  // --- 3 anillos de actividad (derecha) ---
  uint32_t steps = _ctx->fitness ? _ctx->fitness->steps() : 0;
  int      kcal  = _ctx->fitness ? _ctx->fitness->activeCalories() : 0;  // anillo Move = activas
  int      stand = _ctx->fitness ? _ctx->fitness->standHours() : 0;
  float calF   = (cfg::CALORIE_GOAL > 0) ? static_cast<float>(kcal) / cfg::CALORIE_GOAL : 0;
  float stepF  = (_ctx->stepGoal > 0)    ? static_cast<float>(steps) / _ctx->stepGoal : 0;
  float standF = (cfg::STAND_GOAL > 0)   ? static_cast<float>(stand) / cfg::STAND_GOAL : 0;
  drawRings(c, 198, 62, calF, stepF, standF);

  // --- Distancia (abajo) ---
  float km = _ctx->fitness ? _ctx->fitness->distanceMeters() / 1000.0f : 0.0f;
  char dist[16];
  snprintf(dist, sizeof(dist), "%.2f km", km);
  c.setFont(&fonts::Font4);
  c.setTextDatum(bottom_left);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(dist, 6, H - 1);
}

void WatchfaceScreen::drawMinimal(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();

  // Fecha en blanco, arriba-izquierda.
  char fecha[10];
  snprintf(fecha, sizeof(fecha), "%02d %s", t.day, TimeService::monthShort(t.month));
  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_TIME);
  c.drawString(fecha, 6, 4);

  // Batería + BLE arriba-derecha (igual que en las otras esferas).
  drawBattery(c, W - 28, 4, _ctx->power->batteryLevel(), _ctx->power->isCharging());
  drawBluetooth(c, W - 44, 3, _ctx->bleConnected ? cfg::COL_CYAN : cfg::COL_DIM);

  // Hora ENORME (fuente 7 segmentos escalada).
  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font7);
  c.setTextSize(1.6f);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_CYAN);
  c.drawString(hora, W / 2, H / 2 + 8);
  c.setTextSize(1.0f);   // restaurar para el resto de pantallas
}

void WatchfaceScreen::drawDashboard(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  LocalTime t = _ctx->time->now();

  // --- Cabecera: hora (izq) + batería/BLE (der, como en la esfera principal) ---
  char hora[6];
  snprintf(hora, sizeof(hora), "%02d:%02d", t.hour, t.minute);
  c.setFont(&fonts::Font4);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_CYAN);
  c.drawString(hora, 6, 3);

  drawBattery(c, W - 28, 4, _ctx->power->batteryLevel(), _ctx->power->isCharging());
  drawBluetooth(c, W - 44, 3, _ctx->bleConnected ? cfg::COL_CYAN : cfg::COL_DIM);

  c.drawFastHLine(6, 26, W - 12, cfg::COL_DIM);

  // --- Datos: icono (color del dato) + valor ---
  uint32_t steps = _ctx->fitness ? _ctx->fitness->steps() : 0;
  float    km    = _ctx->fitness ? _ctx->fitness->distanceMeters() / 1000.0f : 0.0f;
  int      kcal  = _ctx->fitness ? _ctx->fitness->calories() : 0;
  uint32_t amin  = _ctx->fitness ? _ctx->fitness->activeMinutes() : 0;
  int      stand = _ctx->fitness ? _ctx->fitness->standHours() : 0;

  char vals[5][12];
  snprintf(vals[0], sizeof(vals[0]), "%lu", static_cast<unsigned long>(steps));
  snprintf(vals[1], sizeof(vals[1]), "%.2f", km);
  snprintf(vals[2], sizeof(vals[2]), "%d", kcal);
  snprintf(vals[3], sizeof(vals[3]), "%lu min", static_cast<unsigned long>(amin));
  snprintf(vals[4], sizeof(vals[4]), "%d h", stand);

  const uint16_t vcol[5] = {cfg::COL_RING_STEP, cfg::COL_CYAN, cfg::COL_RING_MOVE,
                            cfg::COL_CYAN, cfg::COL_RING_STAND};

  // Fila 1: pasos, km, kcal (3 columnas). Fila 2: activo, de pie (2 centradas).
  const int r1cols[3] = {W / 6, W / 2, 5 * W / 6};
  const int r2cols[2] = {W / 3, 2 * W / 3};
  const int icoY1 = 46, valY1 = 66;
  const int icoY2 = 96, valY2 = 116;

  c.setTextDatum(middle_center);
  c.setFont(&fonts::Font2);

  icoSteps(c, r1cols[0], icoY1, vcol[0]);
  icoPin(c,   r1cols[1], icoY1, vcol[1]);
  icoFlame(c, r1cols[2], icoY1, vcol[2]);
  icoStopwatch(c, r2cols[0], icoY2, vcol[3]);
  icoPerson(c,    r2cols[1], icoY2, vcol[4]);

  for (int i = 0; i < 3; i++) { c.setTextColor(vcol[i]); c.drawString(vals[i], r1cols[i], valY1); }
  c.setTextColor(vcol[3]); c.drawString(vals[3], r2cols[0], valY2);
  c.setTextColor(vcol[4]); c.drawString(vals[4], r2cols[1], valY2);
}
