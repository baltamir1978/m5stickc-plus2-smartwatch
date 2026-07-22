#include <M5Unified.h>
#include <cstdio>
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

  M5.Speaker.setVolume(255);   // volumen máximo para alarmas / alertas de llamada

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

// Gestiona la alarma: suena a la hora fijada; se para con BtnB (o sola a los 60 s).
// Devuelve true mientras está sonando.
static bool handleAlarm(const LocalTime& t) {
  static bool     ringing = false;
  static uint32_t startMs = 0;
  static int      firedKey = -1;
  static uint32_t lastBeep = 0, lastDraw = 0;
  static bool     blink = false;

  const int nowKey = t.hour * 60 + t.minute;
  const int almKey = settings.alarmHour() * 60 + settings.alarmMin();

  if (settings.alarmOn() && nowKey == almKey) {
    if (firedKey != nowKey && !ringing) { ringing = true; startMs = millis(); firedKey = nowKey; }
  } else if (nowKey != almKey) {
    firedKey = -1;   // rearmar para el próximo día
  }

  if (!ringing) return false;

  if (!power.screenOn()) power.wake();
  power.notifyActivity();

  if (millis() - lastBeep > 800) { lastBeep = millis(); M5.Speaker.tone(3000, 350); }
  if (millis() - lastDraw > 500) {
    lastDraw = millis();
    blink = !blink;
    char ts[6];
    snprintf(ts, sizeof(ts), "%02d:%02d", settings.alarmHour(), settings.alarmMin());
    ui.drawAlarmOverlay(ts, blink);
  }
  if (M5.BtnB.wasPressed() || M5.BtnA.wasPressed()) ringing = false;
  if (millis() - startMs > 60000) ringing = false;
  return true;
}

// Celebración breve al alcanzar un objetivo (melodía ascendente + aviso).
static void handleCelebration(int c) {
  static const char* labels[] = {"", "PASOS", "CALORIAS", "DE PIE"};
  if (c < 1 || c > 3) return;
  if (!power.screenOn()) power.wake();
  power.notifyActivity();
  ui.drawCelebration(labels[c]);
  M5.Speaker.tone(2000, 150); delay(160);
  M5.Speaker.tone(2600, 150); delay(160);
  M5.Speaker.tone(3200, 250); delay(300);
  delay(900);
  ui.forceRedraw();
}

// Aviso de batería baja (dos pitidos + banner), una vez al cruzar el umbral.
static void handleLowBattery() {
  if (!power.screenOn()) power.wake();
  power.notifyActivity();
  ui.drawBanner("BATERIA", "baja", cfg::COL_BAT_LOW);
  M5.Speaker.tone(1200, 300); delay(350);
  M5.Speaker.tone(1000, 300); delay(350);
  delay(700);
  ui.forceRedraw();
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

  // Brillo: máximo en la linterna; si no, automático día/noche.
  if (ui.currentFullBright()) {
    power.setBrightness(255);
  } else {
    bool day = (t.hour >= settings.dayHour() && t.hour < settings.nightHour());
    power.setBrightness(day ? cfg::BRIGHTNESS_DAY : cfg::BRIGHTNESS_NIGHT);
  }

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

  // Alarma (prioridad tras la llamada).
  static bool wasAlarm = false;
  bool nowAlarm = handleAlarm(t);
  if (nowAlarm) { wasAlarm = true; delay(20); return; }
  if (wasAlarm) { wasAlarm = false; ui.forceRedraw(); }

  // Celebración de objetivos (pasos / calorías / de pie).
  if (int celeb = fitness.consumeCelebration()) handleCelebration(celeb);

  // Aviso de batería baja (una vez al cruzar el umbral; se rearma al cargar/subir).
  static bool lowWarned = false;
  int batLvl = power.batteryLevel();
  bool charging = power.isCharging();
  if (!charging && batLvl >= 0 && batLvl <= cfg::BATTERY_WARN_PCT && !lowWarned) {
    lowWarned = true;
    handleLowBattery();
  }
  if (charging || batLvl > cfg::BATTERY_WARN_PCT + 5) lowWarned = false;

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
