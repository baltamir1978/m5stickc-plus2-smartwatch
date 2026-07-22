#pragma once
#include <M5Unified.h>
#include "config.h"

// Iconos de métricas y utilidades dibujados con primitivas, centrados en (cx,cy),
// ~16 px salvo la TV. Compartidos por varias pantallas.
namespace icons {

inline void steps(M5Canvas& c, int cx, int cy, uint16_t col) {   // huellas
  c.fillEllipse(cx - 3, cy - 2, 2, 4, col);
  c.fillCircle(cx - 3, cy + 3, 2, col);
  c.fillEllipse(cx + 3, cy, 2, 4, col);
  c.fillCircle(cx + 3, cy + 5, 2, col);
}

inline void pin(M5Canvas& c, int cx, int cy, uint16_t col) {     // marcador (distancia)
  c.fillCircle(cx, cy - 3, 6, col);
  c.fillTriangle(cx - 5, cy, cx + 5, cy, cx, cy + 8, col);
  c.fillCircle(cx, cy - 3, 2, cfg::COL_BG);
}

inline void flame(M5Canvas& c, int cx, int cy, uint16_t col) {   // llama (calorías)
  c.fillTriangle(cx, cy - 9, cx - 5, cy + 3, cx + 5, cy + 3, col);
  c.fillCircle(cx, cy + 3, 5, col);
  c.fillCircle(cx, cy + 4, 2, cfg::COL_BG);
}

inline void stopwatch(M5Canvas& c, int cx, int cy, uint16_t col) {  // cronómetro (activo)
  c.drawCircle(cx, cy + 1, 7, col);
  c.drawCircle(cx, cy + 1, 6, col);
  c.fillRect(cx - 2, cy - 9, 4, 3, col);
  c.drawLine(cx, cy + 1, cx + 3, cy - 3, col);
}

inline void person(M5Canvas& c, int cx, int cy, uint16_t col) {  // persona (de pie)
  c.fillCircle(cx, cy - 6, 3, col);
  c.fillRoundRect(cx - 2, cy - 2, 4, 9, 2, col);
  c.drawFastHLine(cx - 4, cy + 8, 9, col);
}

inline void bell(M5Canvas& c, int cx, int cy, uint16_t col) {   // campana (alarma)
  c.fillCircle(cx, cy - 9, 2, col);                              // pomo
  c.fillTriangle(cx, cy - 7, cx - 9, cy + 5, cx + 9, cy + 5, col);  // cuerpo
  c.fillRect(cx - 10, cy + 5, 20, 2, col);                       // borde
  c.fillCircle(cx, cy + 9, 2, col);                              // badajo
}

inline void bellSmall(M5Canvas& c, int cx, int cy, uint16_t col) {  // campana para barra de estado
  c.fillCircle(cx, cy - 6, 2, col);                              // pomo
  c.fillTriangle(cx, cy - 5, cx - 6, cy + 3, cx + 6, cy + 3, col);  // cuerpo (más ancho)
  c.fillRect(cx - 7, cy + 3, 14, 2, col);                        // borde (más grueso)
  c.fillCircle(cx, cy + 6, 2, col);                              // badajo
}

inline void star(M5Canvas& c, int cx, int cy, uint16_t col) {   // estrella (celebración)
  c.fillTriangle(cx, cy - 9, cx - 8, cy + 5, cx + 8, cy + 5, col);
  c.fillTriangle(cx, cy + 9, cx - 8, cy - 5, cx + 8, cy - 5, col);
}

// Televisor con antenas (verde) y aspa roja delante (TV-B-Gone). Icono grande.
inline void tv(M5Canvas& c, int cx, int cy) {
  const uint16_t green = cfg::COL_ACCENT;
  const uint16_t red   = cfg::COL_BAT_LOW;
  const int w = 56, h = 38;
  const int x = cx - w / 2, y = cy - h / 2 + 6;

  // Antenas (trazo grueso).
  for (int o = -1; o <= 1; o++) {
    c.drawLine(cx - 1, y + o, cx - 15, y - 15 + o, green);
    c.drawLine(cx + 1, y + o, cx + 15, y - 15 + o, green);
  }
  // Cuerpo (marco grueso: varios roundrect anidados).
  for (int o = 0; o < 3; o++) {
    c.drawRoundRect(x + o, y + o, w - 2 * o, h - 2 * o, 5, green);
  }
  // Patas (gruesas).
  for (int o = -1; o <= 1; o++) {
    c.drawLine(x + 12, y + h + o, x + 6, y + h + 7 + o, green);
    c.drawLine(x + w - 12, y + h + o, x + w - 6, y + h + 7 + o, green);
  }
  // Aspa roja (trazo muy grueso).
  for (int o = -2; o <= 2; o++) {
    c.drawLine(x + 8 + o, y + 7, x + w - 8 + o, y + h - 7, red);
    c.drawLine(x + w - 8 + o, y + 7, x + 8 + o, y + h - 7, red);
    c.drawLine(x + 8, y + 7 + o, x + w - 8, y + h - 7 + o, red);
    c.drawLine(x + w - 8, y + 7 + o, x + 8, y + h - 7 + o, red);
  }
}

}  // namespace icons
