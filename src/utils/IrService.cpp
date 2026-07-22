#include "utils/IrService.h"
#include "config.h"
#include <Arduino.h>
#include <IRsend.h>

void IrService::begin() {
  _tx = new IRsend(cfg::IR_TX_PIN);
  _tx->begin();
}

void IrService::powerOffAll(std::function<void(int, int, const char*)> onStep) {
  if (_tx == nullptr) return;

  int idx = 0;
  const int total = codeCount();
  auto step = [&](const char* brand) {
    ++idx;
    if (onStep) onStep(total, idx, brand);
    delay(80);  // pequeña pausa entre marcas
  };

  _tx->sendSAMSUNG(0xE0E040BFULL, 32);        step("Samsung");
  _tx->sendNEC(0x20DF10EFULL, 32);            step("LG");        // LG usa NEC
  _tx->sendSony(0xA90, 12, 2);                step("Sony12");    // SIRC 12-bit, repetido
  _tx->sendSony(0x750, 15, 2);                step("Sony15");    // SIRC 15-bit (equipos Sony recientes)
  _tx->sendPanasonic(0x4004, 0x100BCBD, 48);  step("Panasonic");
  _tx->sendNEC(0x02FD48B7ULL, 32);            step("Toshiba");   // Toshiba (NEC)
  _tx->sendRC5(0x0C, 13);                     step("Philips5");  // Philips RC5 power
  _tx->sendRC6(0x0C, 20);                     step("Philips6");  // Philips RC6 power (TVs recientes)
}
