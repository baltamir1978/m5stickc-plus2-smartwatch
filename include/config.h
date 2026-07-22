#pragma once
#include <cstdint>

// Configuración global del smartwatch.
namespace cfg {

// --- Pantalla (se usa en horizontal: 240x135) ---
constexpr uint8_t  SCREEN_ROTATION = 3;       // 1 o 3 => landscape (3 = girada 180°)
constexpr uint8_t  BRIGHTNESS      = 90;      // 0-255 (respaldo)
constexpr uint32_t INACTIVITY_MS   = 5000;    // apagar pantalla tras N s (default; editable en Ajustes)

// Brillo automático día/noche (horas configurables en Ajustes).
constexpr uint8_t  BRIGHTNESS_DAY   = 120;    // de día
constexpr uint8_t  BRIGHTNESS_NIGHT = 25;     // de noche
constexpr int      DAY_START_HOUR   = 8;      // hora en que empieza el brillo de día
constexpr int      NIGHT_START_HOUR = 21;     // hora en que empieza el brillo de noche

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

// Metas de los anillos de actividad (estilo Apple).
constexpr int CALORIE_GOAL      = 600;     // kcal activas al día (anillo rojo)
constexpr int STAND_GOAL        = 12;      // horas de pie al día (anillo azul)
constexpr int STAND_STEPS_HOUR  = 20;      // pasos en una hora para contarla como "de pie"

// Umbral de cadencia para considerar un minuto como "activo" (pasos/min).
constexpr uint32_t ACTIVE_CADENCE_SPM = 40;

// Recordatorio de inactividad (solo en horario activo).
constexpr int      ACTIVE_START_HOUR   = 9;
constexpr int      ACTIVE_END_HOUR     = 21;
constexpr uint32_t INACTIVITY_ALERT_MS = 60UL * 60UL * 1000UL;  // 60 min sin pasos

// --- Gestos: girar la muñeca para encender la pantalla ---
// Se detecta el GIRO (giroscopio) que termina con la pantalla mirando al usuario,
// en vez de la orientación absoluta (que falla si ya partías con la pantalla arriba).
constexpr float    WRIST_TURN_DPS      = 120.0f; // °/s: pico de giro para considerar "rotación de muñeca"
constexpr float    WRIST_STILL_DPS     = 40.0f;  // °/s: por debajo = giro terminado
constexpr float    WRIST_MIN_Z         = 0.30f;  // orientación final "mirando" (az del acelerómetro)
constexpr uint32_t WRIST_REFRACTORY_MS = 1000;   // ms mínimos entre disparos

// Log del acelerómetro/giroscopio por serie (para calibrar). Poner en false cuando no se use.
constexpr bool     DEBUG_IMU           = true;

// --- Colores (RGB565) ---
constexpr uint16_t COL_BG      = 0x0000;  // negro
constexpr uint16_t COL_TIME    = 0xFFFF;  // blanco
constexpr uint16_t COL_DATE    = 0xAD55;  // gris claro
constexpr uint16_t COL_ACCENT  = 0x07E0;  // verde
constexpr uint16_t COL_CYAN    = 0x05FF;  // cian (hora)
constexpr uint16_t COL_ORANGE  = 0xFD20;  // naranja (batería media)
constexpr uint16_t COL_BOLT    = 0xFFE0;  // amarillo (rayo de carga)
constexpr uint16_t COL_RING_MOVE  = 0xF9E7; // rojo-rosa (anillo calorías)
constexpr uint16_t COL_RING_STEP  = 0x2FEA; // verde (anillo pasos)
constexpr uint16_t COL_RING_STAND = 0x05FF; // cian (anillo de pie)
constexpr uint16_t COL_BAT_LOW = 0xF800;  // rojo
constexpr uint16_t COL_DIM     = 0x52AA;  // gris oscuro

constexpr int BATTERY_LOW_PCT = 20;       // umbral de batería baja
constexpr int BATTERY_MID_PCT = 40;       // umbral de batería media (naranja)

}  // namespace cfg
