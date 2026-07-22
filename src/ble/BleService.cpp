#include "ble/BleService.h"
#include "ble/CallAlert.h"
#include "core/TimeService.h"
#include <NimBLEDevice.h>
#include <Arduino.h>
#include <cstring>

// --- UUIDs de Apple ---
static const NimBLEUUID ANCS_SVC ("7905F431-B5CE-4E99-A40F-4B1E122D00D0");
static const NimBLEUUID ANCS_NS  ("9FBF120D-6301-42D9-8C58-25E699A21DBD");  // Notification Source (notify)
static const NimBLEUUID ANCS_CP  ("69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9");  // Control Point (write)
static const NimBLEUUID ANCS_DS  ("22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB");  // Data Source (notify)
static const NimBLEUUID CTS_SVC  ((uint16_t)0x1805);                        // Current Time Service
static const NimBLEUUID CTS_CHR  ((uint16_t)0x2A2B);                        // Current Time

// ANCS: CategoryID / EventID / CommandID / AttributeID / ActionID
static constexpr uint8_t CAT_INCOMING_CALL   = 1;
static constexpr uint8_t EVENT_ADDED         = 0;
static constexpr uint8_t EVENT_REMOVED       = 2;
static constexpr uint8_t CMD_GET_ATTRS       = 0x00;
static constexpr uint8_t CMD_PERFORM_ACTION  = 0x02;
static constexpr uint8_t ATTR_TITLE          = 0x01;
static constexpr uint8_t ACTION_NEGATIVE     = 0x01;  // rechazar/colgar

namespace {
// Callbacks del servidor: reenvían al BleService.
class ServerCallbacks : public NimBLEServerCallbacks {
public:
  explicit ServerCallbacks(BleService* owner) : _o(owner) {}
  void onConnect(NimBLEServer* s, NimBLEConnInfo& info) override { _o->handleConnect(s, info); }
  void onDisconnect(NimBLEServer* s, NimBLEConnInfo& info, int reason) override { _o->handleDisconnect(); }
private:
  BleService* _o;
};
}  // namespace

void BleService::begin(TimeService* time, CallAlert* call, const char* deviceName) {
  _time = time;
  _call = call;

  NimBLEDevice::init(deviceName);
  // Emparejamiento con bonding cifrado. "Just Works" (sin passkey) porque el
  // reloj no tiene UI de confirmación; suficiente para ANCS.
  NimBLEDevice::setSecurityAuth(true, false, true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
  NimBLEDevice::setPower(9);

  _server = NimBLEDevice::createServer();
  _server->setCallbacks(new ServerCallbacks(this));
  _server->advertiseOnDisconnect(true);

  NimBLEAdvertising* adv = _server->getAdvertising();
  NimBLEAdvertisementData advData;
  advData.setFlags(0x06);
  advData.addServiceUUID(ANCS_SVC);           // solicita la relación ANCS a iOS
  adv->setAdvertisementData(advData);

  NimBLEAdvertisementData scanData;
  scanData.setName(deviceName);
  adv->setScanResponseData(scanData);
  adv->enableScanResponse(true);
  adv->start();
  Serial.printf("[BLE] Advertising como '%s'. Esperando al iPhone...\n", deviceName);
}

void BleService::handleConnect(NimBLEServer* server, NimBLEConnInfo& info) {
  _client = server->getClient(info);
  _connected = true;
  _state = CONNECTED;
  _lastDiscoveryMs = 0;
  Serial.println("[BLE] iPhone conectado. Descubriendo ANCS/CTS...");
}

void BleService::handleDisconnect() {
  _connected = false;
  _state = IDLE;
  _client = nullptr;
  _ns = _ds = _cp = _cts = nullptr;
  _callUidValid = false;
  _fetchTitle = false;
  if (_call) _call->clear();
  Serial.println("[BLE] Desconectado.");
}

void BleService::update() {
  if (!_connected || _client == nullptr) return;

  if (_state == CONNECTED) {
    if (millis() - _lastDiscoveryMs > 500) {
      _lastDiscoveryMs = millis();
      discover();
    }
    return;
  }

  if (_state == READY) {
    if (_fetchTitle && _cp) {
      _fetchTitle = false;
      requestTitle();
    }
    if (_cts && _time && _time->needsSync() && (millis() - _lastCtsMs > 5000)) {
      _lastCtsMs = millis();
      syncTimeFromCts();
    }
  }
}

void BleService::discover() {
  if (!_client->isConnected()) return;

  // iOS solo expone ANCS tras el bonding; si aún no está, getService devuelve null → reintentar.
  NimBLERemoteService* ancs = _client->getService(ANCS_SVC);
  if (ancs == nullptr) return;

  _ns = ancs->getCharacteristic(ANCS_NS);
  _cp = ancs->getCharacteristic(ANCS_CP);
  _ds = ancs->getCharacteristic(ANCS_DS);
  if (_ns == nullptr || _cp == nullptr || _ds == nullptr) return;

  _ds->subscribe(true, [this](NimBLERemoteCharacteristic*, uint8_t* d, size_t n, bool) {
    onDataSource(d, n);
  });
  _ns->subscribe(true, [this](NimBLERemoteCharacteristic*, uint8_t* d, size_t n, bool) {
    onNotificationSource(d, n);
  });

  // Current Time Service (opcional): puede no estar hasta que iOS lo expone.
  NimBLERemoteService* cts = _client->getService(CTS_SVC);
  if (cts) _cts = cts->getCharacteristic(CTS_CHR);

  _state = READY;
  Serial.println("[BLE] ANCS listo. Avisos de llamada activos.");
}

void BleService::onNotificationSource(const uint8_t* d, size_t len) {
  if (len < 8) return;
  const uint8_t eventID  = d[0];
  const uint8_t category = d[2];
  if (category != CAT_INCOMING_CALL) return;  // solo llamadas entrantes

  if (eventID == EVENT_ADDED) {
    for (int i = 0; i < 4; i++) _uid[i] = d[4 + i];
    _callUidValid = true;
    _call->incoming(nullptr);   // nombre genérico hasta obtener el Title
    _fetchTitle = true;         // el write al Control Point lo hace update()
    Serial.println("[BLE] Llamada entrante!");
  } else if (eventID == EVENT_REMOVED) {
    _call->clear();
    _callUidValid = false;
  }
}

void BleService::onDataSource(const uint8_t* d, size_t len) {
  // Formato: [CommandID][UID(4)] luego atributos: [AttrID][len(2 LE)][datos...]
  if (len < 8) return;
  size_t i = 5;
  while (i + 3 <= len) {
    const uint8_t  attrID = d[i];
    const uint16_t alen   = static_cast<uint16_t>(d[i + 1] | (d[i + 2] << 8));
    i += 3;
    if (i + alen > len) break;
    if (attrID == ATTR_TITLE && alen > 0 && _call->active()) {
      char name[40];
      size_t n = (alen < sizeof(name) - 1) ? alen : sizeof(name) - 1;
      memcpy(name, &d[i], n);
      name[n] = '\0';
      _call->incoming(name);
    }
    i += alen;
  }
}

void BleService::requestTitle() {
  // GetNotificationAttributes: cmd + UID + AttributeID(Title) + maxlen(2 LE)
  uint8_t v[8] = {CMD_GET_ATTRS, _uid[0], _uid[1], _uid[2], _uid[3], ATTR_TITLE, 0x20, 0x00};
  _cp->writeValue(v, sizeof(v), true);
}

void BleService::rejectCall() {
  if (!_callUidValid || _cp == nullptr) return;
  // PerformNotificationAction: cmd + UID + ActionID(negativo = colgar)
  uint8_t v[6] = {CMD_PERFORM_ACTION, _uid[0], _uid[1], _uid[2], _uid[3], ACTION_NEGATIVE};
  _cp->writeValue(v, sizeof(v), true);
}

void BleService::syncTimeFromCts() {
  NimBLEAttValue val = _cts->readValue();
  if (val.length() < 7) return;
  const uint8_t* d = val.data();
  LocalTime t;
  t.year    = d[0] | (d[1] << 8);
  t.month   = d[2];
  t.day     = d[3];
  t.hour    = d[4];
  t.minute  = d[5];
  t.second  = d[6];
  t.weekday = 0;  // TimeService recalcula el día de la semana
  _time->applyPhoneTime(t);
}
