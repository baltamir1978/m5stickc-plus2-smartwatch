#pragma once
#include <cstdint>

// Configuración global del smartwatch.
namespace cfg {

// --- Pantalla (se usa en horizontal: 240x135) ---
constexpr uint8_t  SCREEN_ROTATION = 1;       // 1 o 3 => landscape
constexpr uint8_t  BRIGHTNESS      = 90;      // 0-255
constexpr uint32_t INACTIVITY_MS   = 15000;   // apagar pantalla tras 15 s sin actividad

// --- Energía (light sleep base): frecuencia de CPU activa vs. en reposo ---
constexpr uint32_t CPU_FREQ_ACTIVE = 240;     // MHz con la pantalla encendida
constexpr uint32_t CPU_FREQ_IDLE   = 80;      // MHz con la pantalla apagada (ahorro)

// --- BLE ---
constexpr const char* BLE_DEVICE_NAME = "M5 Smartwatch";

// --- IR (TV-B-Gone) ---
constexpr uint16_t IR_TX_PIN = 19;   // emisor IR del M5StickC Plus 2 (comparte pin con el LED)

// --- Fitness ---
constexpr uint32_t STEP_GOAL_DEFAULT = 10000;

// Perfil de usuario por defecto (editable en runtime desde la pantalla de Ajustes).
constexpr float USER_HEIGHT_CM = 181.0f;   // para estimar la zancada
constexpr float USER_WEIGHT_KG = 100.0f;   // para estimar calorías

// Umbral de cadencia para considerar un minuto como "activo" (pasos/min).
constexpr uint32_t ACTIVE_CADENCE_SPM = 60;

// Recordatorio de inactividad (solo en horario activo).
constexpr int      ACTIVE_START_HOUR   = 9;
constexpr int      ACTIVE_END_HOUR     = 21;
constexpr uint32_t INACTIVITY_ALERT_MS = 60UL * 60UL * 1000UL;  // 60 min sin pasos

// --- Gestos: levantar la muñeca para encender la pantalla ---
// Nota: eje/umbrales dependen del montaje en la muñeca; calibrar con el reloj puesto.
// Se usa el eje Z del acelerómetro (normal a la pantalla): ~+1 g cuando la miras.
constexpr float    WRIST_VIEW_Z        = 0.55f;  // pantalla orientada hacia el usuario
constexpr float    WRIST_DOWN_Z        = 0.25f;  // brazo bajado (rearme del gesto)
constexpr float    WRIST_MOTION_G      = 0.15f;  // desviación mínima de |accel| = hubo movimiento
constexpr uint32_t WRIST_MOTION_WIN_MS = 800;    // ventana de "movimiento reciente"

// --- Colores (RGB565) ---
constexpr uint16_t COL_BG      = 0x0000;  // negro
constexpr uint16_t COL_TIME    = 0xFFFF;  // blanco
constexpr uint16_t COL_DATE    = 0xAD55;  // gris claro
constexpr uint16_t COL_ACCENT  = 0x07E0;  // verde
constexpr uint16_t COL_BAT_LOW = 0xF800;  // rojo
constexpr uint16_t COL_DIM     = 0x52AA;  // gris oscuro

constexpr int BATTERY_LOW_PCT = 20;       // umbral de batería baja

}  // namespace cfg
