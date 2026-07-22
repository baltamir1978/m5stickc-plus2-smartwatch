#pragma once
#include <cstdint>
#include <cstddef>

// Forward declarations (los tipos NimBLE se incluyen en el .cpp).
class NimBLEServer;
class NimBLEClient;
class NimBLEConnInfo;
class NimBLERemoteCharacteristic;
class TimeService;
class CallAlert;

// Servicio BLE: periférico que se empareja con el iPhone y, sobre la misma
// conexión, actúa como cliente GATT para:
//   - ANCS: avisar SOLO de llamadas entrantes (y colgarlas con rejectCall()).
//   - CTS : leer la hora del iPhone y sincronizar el RTC.
//
// iOS conecta como central; nosotros descubrimos ANCS/CTS en el iPhone.
class BleService {
public:
  void begin(TimeService* time, CallAlert* call, const char* deviceName);
  void update();               // llamar en cada iteración del loop
  bool connected() const { return _connected; }
  void rejectCall();           // cuelga la llamada actual (BtnB)

  // Invocados por los callbacks del servidor (no llamar directamente).
  void handleConnect(NimBLEServer* server, NimBLEConnInfo& info);
  void handleDisconnect();

  // Invocados por los callbacks de notificación GATT.
  void onNotificationSource(const uint8_t* data, size_t len);
  void onDataSource(const uint8_t* data, size_t len);

private:
  enum State { IDLE, CONNECTED, READY };

  void discover();
  void requestTitle();
  void syncTimeFromCts();

  TimeService* _time = nullptr;
  CallAlert*   _call = nullptr;

  NimBLEServer* _server = nullptr;
  NimBLEClient* _client = nullptr;
  NimBLERemoteCharacteristic* _ns  = nullptr;  // Notification Source
  NimBLERemoteCharacteristic* _ds  = nullptr;  // Data Source
  NimBLERemoteCharacteristic* _cp  = nullptr;  // Control Point
  NimBLERemoteCharacteristic* _cts = nullptr;  // Current Time

  volatile bool _connected = false;
  volatile int  _state = IDLE;
  volatile bool _fetchTitle = false;
  volatile bool _callUidValid = false;
  volatile uint8_t _uid[4] = {0, 0, 0, 0};

  uint32_t _lastDiscoveryMs = 0;
  uint32_t _lastCtsMs = 0;
};
