#include "UtilitiesScreen.h"
#include "config.h"

void UtilitiesScreen::draw(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  c.setFont(&fonts::Font4);
  c.setTextDatum(middle_center);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString("TV-B-Gone", W / 2, 34);

  c.setFont(&fonts::Font2);
  c.setTextColor(cfg::COL_DATE);
  c.drawString("Apaga cualquier TV", W / 2, H / 2 + 6);

  c.setTextColor(cfg::COL_TIME);
  c.drawString("BtnB: emitir IR", W / 2, H - 16);
}

void UtilitiesScreen::onButtonB() {
  _ctx->tvbgoneRequested = true;   // lo ejecuta el loop principal (envío bloqueante)
}
