#pragma once
#include <M5Unified.h>
#include <Preferences.h>
#include <cstdint>

// Hora local extraída del RTC BM8563 (persiste entre reinicios).
struct LocalTime {
  int year, month, day;
  int weekday;   // 0=Domingo .. 6=Sábado
  int hour, minute, second;
};

// La fuente de verdad es el iPhone (BLE Current Time Service). El RTC BM8563
// mantiene la hora entre sincronizaciones y reinicios. Política de sincronización:
//   - En la primera conexión BLE tras arrancar (para tener hora buena cuanto antes).
//   - Una vez al día a partir de las 04:00 (si no hay móvil, queda pendiente ese día).
class TimeService {
public:
  // Carga el estado de sincronización de NVS. Si el RTC no tiene hora válida,
  // lo inicializa con la fecha/hora de compilación como respaldo.
  void begin();

  LocalTime now();

  // ¿Toca sincronizar con el móvil ahora? (lo consulta la capa BLE, Hito 4)
  bool needsSync();

  // Escribe una hora recibida del iPhone en el RTC y marca el día como sincronizado.
  void applyPhoneTime(const LocalTime& t);

  // Abreviaturas en español (ASCII, sin acentos para la fuente del TFT).
  static const char* weekdayShort(int wd);   // "LUN", "MAR", ...
  static const char* monthShort(int m);      // "ENE", "FEB", ...

private:
  void markSynced();
  int  todayYmd();   // aaaammdd de la fecha actual del RTC

  Preferences _prefs;
  uint32_t    _lastSyncYmd = 0;   // aaaammdd de la última sincronización
  bool        _neverSynced = true;
};
