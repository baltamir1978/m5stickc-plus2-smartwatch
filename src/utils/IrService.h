#pragma once
#include <functional>

class IRsend;

// Emisor IR estilo "TV-B-Gone": emite una secuencia de códigos POWER de varias
// marcas para apagar (o encender) el televisor que tengas delante.
//
// NOTA: set inicial de marcas mayoritarias (Samsung, LG, Sony, Panasonic,
// Philips). No es la base completa del TV-B-Gone original; ampliar añadiendo
// entradas en powerOffAll().
class IrService {
public:
  void begin();
  int  codeCount() const { return 8; }

  // Emite todos los códigos en secuencia. onStep(total, idx, marca) se invoca
  // tras cada envío para que la UI muestre el progreso.
  void powerOffAll(std::function<void(int total, int idx, const char* brand)> onStep = nullptr);

private:
  IRsend* _tx = nullptr;
};
