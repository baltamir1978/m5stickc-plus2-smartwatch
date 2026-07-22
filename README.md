# M5 Smartwatch — M5StickC Plus 2

Firmware que convierte un **M5StickC Plus 2** en un smartwatch: reloj con varias
esferas, contapasos y métricas de actividad, gesto de levantar la muñeca, aviso de
**llamadas entrantes del iPhone** por BLE, sincronización de hora desde el móvil y un
**TV-B-Gone** por infrarrojos.

> Proyecto personal basado en [M5Unified](https://github.com/m5stack/M5Unified),
> [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) e
> [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266).

---

## Características

- **Reloj** con 3 esferas digitales conmutables (persisten en memoria):
  - **CLEAN** — hora grande, fecha, estado y pasos.
  - **MINIMAL** — solo la hora, enorme.
  - **DASHBOARD** — hora + rejilla de pasos, distancia y calorías.
- **Fitness** — contapasos (acelerómetro MPU6886), distancia, calorías, minutos
  activos y aviso de inactividad. Reinicio diario a medianoche; los pasos del día
  sobreviven a reinicios.
- **Wrist-raise** — al levantar la muñeca se enciende la pantalla.
- **Llamadas entrantes (iPhone / ANCS)** — overlay a pantalla completa con el nombre
  de quien llama, pitido y opción de **colgar** desde el reloj.
- **Hora automática (iPhone / CTS)** — sincroniza el RTC al conectar y cada día a las 04:00.
- **TV-B-Gone** — emite códigos POWER de varias marcas por el emisor IR para apagar TVs.
- **Ajustes** — altura, peso y objetivo de pasos, editables desde el reloj y guardados
  en memoria.
- **Ahorro de energía** — pantalla que se apaga por inactividad y CPU que baja a 80 MHz
  en reposo.

## Hardware

| Elemento     | Detalle                                            |
|--------------|----------------------------------------------------|
| MCU          | ESP32-PICO-V3-02 (WiFi + BLE), 8 MB flash          |
| Pantalla     | TFT 1.14" 135×240 (se usa en horizontal, 240×135)  |
| IMU          | MPU6886 (6 ejes) — pasos y gestos                  |
| RTC          | BM8563 — hora persistente                          |
| IR           | Emisor infrarrojo en GPIO19                        |
| Buzzer       | Avisos y alarmas                                    |
| Batería      | LiPo 200 mAh                                        |

## Uso (botones)

El reloj se maneja con tres botones: **BtnA** (frontal, el grande), **BtnB** (lateral)
y **PWR** (arriba).

| Acción | Resultado |
|--------|-----------|
| **BtnA** | Cambia de pantalla: Reloj → Fitness → Utilidades → Ajustes → … |
| **PWR** (clic) | Apaga / enciende la pantalla |
| Pantalla apagada + cualquier botón o **levantar la muñeca** | Enciende la pantalla |

Acciones de **BtnB** según la pantalla:

| Pantalla | BtnB (clic) | BtnB (mantener) |
|----------|-------------|-----------------|
| **Reloj** | Cambia de esfera | — |
| **Utilidades** | Lanza el TV-B-Gone | — |
| **Ajustes** | Sube el valor del campo | Pasa al siguiente campo |
| **Llamada entrante** | Cuelga | — |

## Emparejar el iPhone

El reloj usa **ANCS** (Apple Notification Center Service). iOS suele **no listar** los
periféricos BLE "caseros" en *Ajustes → Bluetooth*, así que el emparejamiento se hace
mejor desde una app de escaneo BLE que inicie la conexión y el *bonding*.

### Con LightBlue (recomendado)

1. Instala **[LightBlue](https://apps.apple.com/app/lightblue/id557428110)** desde la App Store
   (alternativa: **nRF Connect**).
2. Enciende el reloj (el BLE arranca solo y empieza a anunciarse).
3. Abre LightBlue → pestaña **Scan/Peripherals** → busca **"M5 Smartwatch"** en la lista.
4. Pulsa **Connect**. Al conectar, el reloj solicita el emparejamiento cifrado (*bonding*)
   y iOS mostrará el aviso de **"Solicitud de emparejamiento Bluetooth"** → **Emparejar**.
5. Hecho: iOS ya entrega las **llamadas entrantes** y la **hora** al reloj automáticamente.
   Puedes cerrar LightBlue; el vínculo queda guardado.

> **Comprobación:** con el monitor serie abierto (`pio device monitor -b 115200`) verás
> `[BLE] Advertising...`, luego `[BLE] iPhone conectado...` y `[BLE] ANCS listo` cuando
> todo funciona.

### Notas

- Que **no aparezca en Ajustes → Bluetooth** es normal: no significa que esté roto.
  Verifícalo siempre con LightBlue/nRF Connect.
- Si tras reflashear el firmware falla la reconexión, haz **"Olvidar este dispositivo"**
  en Ajustes → Bluetooth (si aparece) y vuelve a emparejar desde LightBlue.

## Compilar y flashear

Requiere [PlatformIO](https://platformio.org/).

```bash
# Compilar
pio run

# Compilar, flashear por USB-C y abrir el monitor serie
pio run -t upload -t monitor
```

La placa se define localmente en `boards/m5stick-c-plus2.json` (variante
`m5stack_stickc_plus2`, 8 MB de flash), porque el board estándar `m5stick-c` apunta a
una variante que no existe en el core moderno de arduino-esp32.

## Estructura del proyecto

```
platformio.ini              Configuración de PlatformIO
boards/                     Definición de placa (Plus 2, 8 MB)
include/config.h            Pines, constantes y parámetros ajustables
src/
  main.cpp                  Bucle principal y orquestación
  core/     PowerManager · TimeService · Settings
  sensors/  MotionService · StepDetector · GestureDetector
  fitness/  FitnessTracker
  ble/      BleService · CallAlert     (ANCS + CTS)
  utils/    IrService                  (TV-B-Gone)
  ui/       UiManager · Screen · screens/*
```

Detalle de arquitectura y decisiones de diseño en [`ARCHITECTURE.md`](ARCHITECTURE.md).

## Ajustes rápidos (`include/config.h`)

- `BRIGHTNESS`, `INACTIVITY_MS` — brillo y tiempo hasta apagar pantalla.
- `USER_HEIGHT_CM`, `USER_WEIGHT_KG`, `STEP_GOAL_DEFAULT` — perfil por defecto.
- `WRIST_*` — umbrales del gesto de levantar la muñeca.
- `ACTIVE_CADENCE_SPM`, `INACTIVITY_ALERT_MS` — actividad e inactividad.
- `CPU_FREQ_ACTIVE` / `CPU_FREQ_IDLE` — frecuencias de CPU (energía).

## Limitaciones y notas

- La autonomía es modesta (batería de 200 mAh); el BLE conectado es el mayor consumo.
- El contapasos y los umbrales de wrist-raise pueden necesitar ajuste según cómo se
  lleve el reloj.
- El TV-B-Gone incluye un conjunto de marcas mayoritarias (Samsung, LG, Sony,
  Panasonic, Philips, Toshiba); ampliable en `src/utils/IrService.cpp`.
- Las notificaciones BLE están limitadas **a propósito** a las llamadas entrantes.
