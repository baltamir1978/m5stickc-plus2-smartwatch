#include <M5Unified.h>
#include "config.h"
#include "core/PowerManager.h"
#include "core/TimeService.h"
#include "core/Settings.h"
#include "fitness/FitnessTracker.h"
#include "sensors/MotionService.h"
#include "sensors/EnvSensor.h"
#include "ble/CallAlert.h"
#include "ble/BleService.h"
#include "utils/IrService.h"
#include "ui/UiManager.h"

static PowerManager   power;
static TimeService    timeSvc;
static Settings       settings;
static FitnessTracker fitness;
static MotionService  motion;
static CallAlert      call;
static EnvSensor      env;
static BleService     ble;
static IrService      ir;
static UiManager      ui;
static AppContext     ctx;

void setup() {
  auto m5cfg = M5.config();
  M5.begin(m5cfg);

  Serial.begin(115200);
  Serial.println("\n[BOOT] M5 Smartwatch iniciando...");

  power.begin();
  timeSvc.begin();
  settings.begin();
  M5.Display.setRotation(settings.rotation());   // orientación elegida en Ajustes
  fitness.begin(&settings);
  motion.begin(&fitness);
  env.begin();   // sondea el ENV III en el puerto Grove

  ctx.time     = &timeSvc;
  ctx.power    = &power;
  ctx.settings = &settings;
  ctx.fitness  = &fitness;
  ctx.call     = &call;
  ctx.env      = &env;
  ctx.stepGoal = settings.stepGoal();

  ir.begin();

  ui.begin(&ctx);
  ui.forceRedraw();

  // BLE al final: emparejamiento con iPhone (ANCS llamadas + CTS hora).
  ble.begin(&timeSvc, &call, cfg::BLE_DEVICE_NAME);
}

// Gestiona la llamada entrante: pantalla encendida, overlay parpadeante y pitido.
// Devuelve true si hay llamada en curso (el loop no procesa el resto).
static bool handleCall() {
  static uint32_t lastBeep = 0;
  static uint32_t lastDraw = 0;
  static bool     blink = false;

  // En modo ahorro no se muestran las llamadas.
  if (settings.bleSaver()) {
    if (call.active()) call.clear();
    return false;
  }

  if (!call.active()) return false;

  if (!power.screenOn()) power.wake();
  power.notifyActivity();

  if (millis() - lastBeep > 1500) {
    lastBeep = millis();
    M5.Speaker.tone(2000, 400);
  }
  if (millis() - lastDraw > 500) {
    lastDraw = millis();
    blink = !blink;
    ui.drawCallOverlay(call.caller(), blink);
  }

  if (M5.BtnB.wasPressed()) ble.rejectCall();
  return true;
}

void loop() {
  M5.update();
  power.update();

  // Sensores + fitness (siempre, aunque la pantalla esté apagada).
  motion.update();
  env.update();
  LocalTime t = timeSvc.now();
  fitness.update(t);
  ctx.steps    = fitness.steps();
  ctx.stepGoal = settings.stepGoal();
  power.setInactivityMs(settings.screenTimeoutMs());
  motion.setFlipped(settings.flipped());

  // Brillo automático día/noche según la hora y las horas configuradas.
  bool day = (t.hour >= settings.dayHour() && t.hour < settings.nightHour());
  power.setBrightness(day ? cfg::BRIGHTNESS_DAY : cfg::BRIGHTNESS_NIGHT);

  // BLE: en modo ahorro se apaga salvo cuando toca sincronizar la hora (1x/día).
  bool bleWanted = !settings.bleSaver() || timeSvc.needsSync();
  ble.setActive(bleWanted);
  ble.update();
  ctx.bleConnected = ble.connected();

  // Llamada entrante tiene prioridad sobre el resto de la UI.
  static bool wasCall = false;
  bool nowCall = handleCall();
  if (nowCall) { wasCall = true; delay(20); return; }
  if (wasCall) { wasCall = false; ui.forceRedraw(); }  // al colgar, restaurar pantalla

  // Aviso de inactividad: doble pitido.
  if (fitness.consumeInactivityAlert()) {
    M5.Speaker.tone(2000, 150);
    delay(180);
    M5.Speaker.tone(2000, 150);
  }

  // Wrist-raise: al levantar la muñeca, enciende la pantalla.
  if (motion.consumeWristRaise()) {
    if (!power.screenOn()) {
      power.wake();
      ui.forceRedraw();
    } else {
      power.notifyActivity();
    }
  }

  // BtnA: despierta la pantalla si está apagada; si no, cambia de pantalla.
  if (M5.BtnA.wasPressed()) {
    if (!power.screenOn()) {
      power.wake();
      ui.forceRedraw();
    } else {
      power.notifyActivity();
      ui.nextScreen();
    }
  }

  // BtnB: mantener = acción terciaria; clic corto = acción secundaria.
  if (M5.BtnB.wasHold()) {
    power.notifyActivity();
    if (!power.screenOn()) {
      power.wake();
      ui.forceRedraw();
    } else {
      ui.onButtonBHold();
    }
  } else if (M5.BtnB.wasClicked()) {
    power.notifyActivity();
    if (!power.screenOn()) {
      power.wake();
      ui.forceRedraw();
    } else {
      ui.onButtonB();
    }
  }

  // TV-B-Gone: lo solicita la pantalla de utilidades (BtnB). Envío bloqueante (~0,5 s).
  if (ctx.tvbgoneRequested) {
    ctx.tvbgoneRequested = false;
    power.notifyActivity();
    ir.powerOffAll(
      [](int total, int idx, const char* brand) { ui.drawIrProgress(brand, idx, total); },
      []() { M5.update(); return static_cast<bool>(M5.BtnB.wasPressed()); });  // BtnB otra vez = parar
    ui.forceRedraw();
  }

  // Botón de power (clic corto): alterna apagado/encendido de pantalla.
  if (M5.BtnPWR.wasClicked()) {
    if (power.screenOn()) {
      power.sleepScreen();
    } else {
      power.wake();
      ui.forceRedraw();
    }
  }

  ui.update();
  delay(20);
}
