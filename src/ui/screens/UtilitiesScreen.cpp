#include "UtilitiesScreen.h"
#include "../Icons.h"
#include "config.h"

void UtilitiesScreen::draw(M5Canvas& c) {
  const int W = c.width();
  const int H = c.height();
  c.fillScreen(cfg::COL_BG);

  c.setFont(&fonts::Font2);
  c.setTextDatum(top_left);
  c.setTextColor(cfg::COL_ACCENT);
  c.drawString("TV-B-Gone", 6, 4);

  // Icono: tele con antenas (verde) y aspa roja delante.
  icons::tv(c, W / 2, H / 2);

  c.setTextColor(cfg::COL_TIME);
  c.setTextDatum(bottom_center);
  c.drawString("BtnB: emitir", W / 2, H - 4);
}

void UtilitiesScreen::onButtonB() {
  _ctx->tvbgoneRequested = true;   // lo ejecuta el loop principal (envío bloqueante)
}
