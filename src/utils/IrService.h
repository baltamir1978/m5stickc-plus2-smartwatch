#pragma once
#include <functional>

class IRsend;

// Emisor IR estilo "TV-B-Gone": emite una secuencia de códigos POWER de varias
// marcas para apagar (o encender) el televisor que tengas delante.
//
// Base de datos europea del TV-B-Gone original (Mitch Altman / Ken Shirriff),
// tabla EUpowerCodes[]. Los códigos comprimidos se descomprimen a arrays raw
// mark/space en microsegundos y se emiten con IRsend::sendRaw(). Ver
// src/utils/TvbGoneEU.h para la fuente y los detalles de transcripción.
class IrService {
public:
  void begin();
  int  codeCount() const;

  // Emite todos los códigos en secuencia. onStep(total, idx, marca) se invoca
  // tras cada envío para que la UI muestre el progreso.
  void powerOffAll(std::function<void(int total, int idx, const char* brand)> onStep = nullptr);

private:
  IRsend* _tx = nullptr;
};
