#include "UiManager.h"
#include "screens/WatchfaceScreen.h"
#include "screens/FitnessScreen.h"
#include "screens/UtilitiesScreen.h"
#include "screens/SettingsScreen.h"
#include "config.h"
#include <cstdio>

void UiManager::begin(AppContext* ctx) {
  _ctx = ctx;

  _canvas.setColorDepth(16);
  _canvas.createSprite(M5.Display.width(), M5.Display.height());

  // BtnA cicla entre estas pantallas.
  _screens.push_back(new WatchfaceScreen(ctx));
  _screens.push_back(new FitnessScreen(ctx));
  _screens.push_back(new UtilitiesScreen(ctx));
  _screens.push_back(new SettingsScreen(ctx));
  _current = 0;
  _screens[_current]->onEnter();
}

void UiManager::update() {
  if (!_ctx->power->screenOn()) return;
  // La hora refresca una vez por minuto; los pasos, en cuanto cambian, para que
  // la watchface y la pantalla de fitness se vean vivas al caminar.
  int minute = _ctx->time->now().minute;
  if (minute != _lastMinute || _ctx->steps != _lastSteps) {
    _lastMinute = minute;
    _lastSteps = _ctx->steps;
    render();
  }
}

void UiManager::forceRedraw() {
  _lastMinute = _ctx->time->now().minute;
  _lastSteps = _ctx->steps;
  render();
}

void UiManager::render() {
  _screens[_current]->draw(_canvas);
  _canvas.pushSprite(0, 0);
}

void UiManager::nextScreen() {
  _current = (_current + 1) % _screens.size();
  _screens[_current]->onEnter();
  forceRedraw();
}

void UiManager::onButtonA() {
  _screens[_current]->onButtonA();
  forceRedraw();
}

void UiManager::onButtonB() {
  _screens[_current]->onButtonB();
  forceRedraw();
}

void UiManager::onButtonBHold() {
  _screens[_current]->onButtonBHold();
  forceRedraw();
}

void UiManager::drawCallOverlay(const char* caller, bool blink) {
  const int W = _canvas.width();
  const int H = _canvas.height();
  _canvas.fillScreen(blink ? cfg::COL_BAT_LOW : cfg::COL_BG);

  _canvas.setTextColor(cfg::COL_TIME);
  _canvas.setFont(&fonts::Font4);
  _canvas.setTextDatum(middle_center);
  _canvas.drawString("LLAMADA", W / 2, 28);

  _canvas.setFont(&fonts::Font2);
  _canvas.drawString(caller, W / 2, H / 2 + 6);

  _canvas.setTextColor(cfg::COL_DATE);
  _canvas.drawString("BtnB: colgar", W / 2, H - 14);

  _canvas.pushSprite(0, 0);
}

void UiManager::drawIrProgress(const char* brand, int idx, int total) {
  const int W = _canvas.width();
  const int H = _canvas.height();
  _canvas.fillScreen(cfg::COL_BG);

  _canvas.setFont(&fonts::Font4);
  _canvas.setTextDatum(middle_center);
  _canvas.setTextColor(cfg::COL_ACCENT);
  _canvas.drawString("TV-B-Gone", W / 2, 30);

  _canvas.setFont(&fonts::Font2);
  _canvas.setTextColor(cfg::COL_TIME);
  char s[24];
  snprintf(s, sizeof(s), "%s (%d/%d)", brand, idx, total);
  _canvas.drawString(s, W / 2, H / 2 + 6);

  const int barX = 8, barY = H - 16, barW = W - 16, barH = 10;
  _canvas.drawRoundRect(barX, barY, barW, barH, 3, cfg::COL_DIM);
  int fillW = (total > 0) ? (barW - 2) * idx / total : 0;
  if (fillW > 0) _canvas.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, cfg::COL_ACCENT);

  _canvas.pushSprite(0, 0);
}
