# Smartwatch M5StickC Plus 2 — Arquitectura

Proyecto de firmware para convertir un **M5StickC Plus 2** en un smartwatch
con contapasos/fitness, gestos y aviso de llamadas entrantes desde un iPhone.

## 1. Hardware objetivo

| Elemento        | Chip / detalle                          | Uso en el reloj                                   |
|-----------------|-----------------------------------------|---------------------------------------------------|
| MCU             | ESP32-PICO-V3-02 (2 núcleos, 240 MHz)   | Lógica + WiFi + BLE                               |
| Flash / PSRAM   | 8 MB Flash · 2 MB PSRAM                  | Firmware, NVS, buffers                            |
| Pantalla        | TFT 1.14" ST7789V2 · 135×240 px color   | Watchface + UI (se usa en horizontal: 240×135)   |
| IMU 6 ejes      | MPU6886                                  | Pasos, gestos, wrist-raise                        |
| Micrófono       | SPM1423 (PDM)                            | (reservado, futuro)                               |
| RTC             | BM8563                                   | Hora persistente entre reinicios                  |
| Buzzer          | pasivo                                   | Alarmas, aviso de llamada                         |
| IR TX           | LED IR                                   | (reservado, futuro)                               |
| LED             | GPIO19                                   | Parpadeo en llamada / notificación                |
| Botones         | BtnA (frontal), BtnB (lateral), Power    | Navegación                                        |
| Batería         | LiPo interna 200 mAh                     | Autonomía modesta → gestión de energía clave      |

> **Nota:** el Plus 2 **NO** lleva el AXP192 del StickC original. La batería se lee
> por ADC y la retención de alimentación se hace con el pin *hold* (GPIO4).

**No disponible:** GPS, pulsómetro/SpO2, barómetro, táctil, conectividad móvil propia.

## 2. Stack de software

- **PlatformIO** + framework **Arduino-ESP32**.
- **M5Unified** — HAL de pantalla, IMU, botones, power, buzzer, mic.
- **NimBLE-Arduino** — stack BLE ligero (necesario para ANCS; Bluedroid consume demasiado).
- Persistencia en **NVS** (Preferences) con escritura diferida.
- Concurrencia **FreeRTOS**: NimBLE en Core 0; app + sensores en Core 1.

`platformio.ini` debe forzar flash de 8 MB (`board_upload.flash_size = 8MB`) y
tabla de particiones con NVS + OTA (el board `m5stick-c` viene a 4 MB por defecto).

## 3. Capas

```
┌─────────────────────────────────────────────┐
│  UI (state machine de pantallas + overlay)    │
├─────────────────────────────────────────────┤
│  Servicios: Fitness · CallAlert · Time        │
├─────────────────────────────────────────────┤
│  Sensores: Motion/Steps/Gestos  ·  BLE/ANCS   │
├─────────────────────────────────────────────┤
│  Core: PowerManager · Settings(NVS) · RTC     │
├─────────────────────────────────────────────┤
│  HAL → M5Unified (MPU6886, ST7789, BM8563…)   │
└─────────────────────────────────────────────┘
```

## 4. Módulos

### Core
- **`PowerManager`** — brillo, apagado de pantalla por inactividad, *light sleep*,
  lectura de batería por ADC, pin *hold* (GPIO4) y botón power.
- **`TimeService`** — mantiene la hora en el RTC BM8563. La **fuente de verdad es el iPhone**
  vía **BLE Current Time Service (CTS)**: se sincroniza al conectar por primera vez tras
  arrancar y luego **una vez al día a partir de las 04:00**. Si a esa hora no hay móvil, la
  sincronización queda pendiente y se realiza en cuanto vuelve la conexión ese día. El último
  día sincronizado se guarda en NVS (aguanta reinicios). Respaldo: hora de compilación si el
  RTC arranca sin hora válida.
- **`Settings`** — perfil de usuario (altura, peso, zancada, objetivo de pasos),
  esfera elegida, ajustes. Persistencia en NVS con escritura diferida.

### Sensores / Fitness
- **`MotionService`** — muestrea el MPU6886 a ~25–50 Hz en tarea dedicada.
- **`StepDetector`** — magnitud del acelerómetro → filtro paso-bajo → detección de
  picos con umbral dinámico y periodo refractario (~250 ms) contra dobles conteos.
- **`GestureDetector`** — *wrist-raise* (encender pantalla al levantar la muñeca) y,
  más adelante, doble golpe / giro para navegar.
- **`FitnessTracker`** — agrega pasos → **distancia** (zancada ≈ altura × 0,415),
  **calorías** (peso × distancia × factor), **minutos activos** (cadencia > umbral) e
  **inactividad** (aviso si 0 pasos durante X min en horario activo). Reset diario a
  medianoche por RTC.

### BLE / Aviso de llamada (solo llamadas entrantes)
- **`AncsClient`** — *advertising* con UUID solicitado de ANCS + **bonding cifrado**
  (obligatorio en iOS), descubre el servicio ANCS del iPhone y se suscribe a
  *Notification Source*. Filtra por `categoryID == IncomingCall` y pide al
  *Data Source* el título (nombre/número de quien llama).
- **`TimeSyncClient`** — sobre el **mismo** vínculo BLE ya emparejado, lee el
  **Current Time Service (CTS, 0x1805 / 0x2A2B)** que expone iOS y llama a
  `TimeService::applyPhoneTime()`. Se dispara cuando `TimeService::needsSync()` lo indica.
- **`CallAlert`** — al detectar llamada entrante: **overlay a pantalla completa** con
  el nombre/número + **buzzer** + **LED parpadeando**. iOS envía *Notification Removed*
  al contestar/rechazar/colgar → el overlay se cierra solo.
  - *Opcional:* **BtnB = rechazar** vía ANCS *Perform Notification Action*.

### Utilidades (IR)
- **`IrService` / `TvBGone`** — usa el **emisor IR** del Stick (LED IR integrado) para
  emitir la secuencia de códigos de apagado de **TV-B-Gone** (barrido de códigos POWER de
  múltiples fabricantes) y apagar casi cualquier televisor. Se lanza desde una pantalla de
  utilidades con BtnB. Los códigos se almacenan en flash (tablas de la base de datos
  TV-B-Gone). Base: librería `IRremoteESP8266` (compatible ESP32) o emisión por RMT.

### UI
- **`UiManager`** — máquina de estados de pantallas.
  Navegación: **BtnA** = ciclar · **BtnB** = seleccionar/acción · **Power** = atrás / dormir.
- Render con **`M5Canvas`** (sprite en RAM) para evitar parpadeo en 240×135.
- Pantallas: `Watchface` · `Fitness` · `Utilidades` (TV-B-Gone) · `Menu` · `Settings`.
- `CallAlertOverlay` — capa que se dibuja por encima de cualquier pantalla.

## 5. Estructura de ficheros

```
platformio.ini
include/config.h              // pines, constantes, feature flags
src/
  main.cpp                    // setup(): init servicios + arranque de tareas
  core/    PowerManager  TimeService  Settings
  sensors/ MotionService  StepDetector  GestureDetector
  fitness/ FitnessTracker
  ble/     AncsClient  CallAlert  TimeSyncClient
  utils/   IrService  TvBGone
  ui/      UiManager  Screen(base)  screens/*
```

## 6. Consideraciones

- **Batería 200 mAh**: BLE conectado es el mayor consumo. Mitigación: intervalos de
  conexión largos, pantalla apagada por defecto + wrist-raise, brillo bajo.
  Autonomía esperada: horas, no días.
- **ANCS** es la parte más laboriosa (bonding + GATT + parseo). Se aborda como hito
  propio tras la base fitness.
- **RAM**: con NimBLE vamos holgados; vigilar heap con sprites.

## 7. Hitos

1. **Esqueleto** ✅ — PlatformIO + M5Unified + PowerManager + RTC + UiManager + watchface.
2. **Fitness** ✅ — MotionService + StepDetector + FitnessTracker + pantalla de pasos.
3. **Gestos** ✅ — wrist-raise para encender pantalla.
4. **Aviso de llamada + hora** ✅ — ANCS + CallAlert (overlay + buzzer) y sincronización de
   hora vía CTS con el iPhone. (Compila; pendiente validar bonding/llamadas con el iPhone.)
5. **TV-B-Gone** ✅ — IrService (IR en GPIO19) con códigos POWER y pantalla de utilidades.
6. **Pulido** — esferas conmutables (CLEAN/MINIMAL/DASHBOARD, BtnB, persistidas) ✅ compila;
   pendientes: ajustes de perfil editables y gestión fina de energía (light sleep) — requieren
   pruebas en hardware para calibrar consumo.
