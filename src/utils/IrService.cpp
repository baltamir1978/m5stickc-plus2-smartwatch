#include "utils/IrService.h"
#include "utils/TvbGoneEU.h"
#include "config.h"
#include <Arduino.h>
#include <IRsend.h>

void IrService::begin() {
  _tx = new IRsend(cfg::IR_TX_PIN);
  _tx->begin();
}

int IrService::codeCount() const {
  return tvbgone_eu::EU_CODE_COUNT;
}

void IrService::powerOffAll(std::function<void(int, int, const char*)> onStep) {
  if (_tx == nullptr) return;

  const int total = tvbgone_eu::EU_CODE_COUNT;
  static char label[16];

  for (int i = 0; i < total; ++i) {
    const tvbgone_eu::EuCode& c = tvbgone_eu::EU_CODES[i];

    // Copiar el array desde PROGMEM a un buffer temporal en RAM: sendRaw()
    // espera un puntero a RAM. El código EU más largo son unos cientos de
    // pares, así que cabe holgadamente en la pila.
    uint16_t buf[512];
    uint16_t len = c.len;
    if (len > 512) len = 512;  // salvaguarda; ningún código EU se acerca a esto
    for (uint16_t j = 0; j < len; ++j) {
      buf[j] = pgm_read_word(&c.raw[j]);
    }

    _tx->sendRaw(buf, len, c.khz);

    if (onStep) {
      snprintf(label, sizeof(label), "EU %d/%d", i + 1, total);
      onStep(total, i + 1, label);
    }
    delay(60);  // pequeña pausa entre códigos
  }
}
