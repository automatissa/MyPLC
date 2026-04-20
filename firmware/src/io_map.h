#pragma once
// ============================================================================
//  io_map.h — SEUL FICHIER À ÉDITER dans le firmware ESP32
//
//  L'ESP32 se connecte à la Raspberry Pi (Modbus TCP Server) et :
//    - Lit les GPIO → écrit dans les VAR_INPUT de la RPi  (FC06)
//    - Lit les VAR_OUTPUT de la RPi → pilote les GPIO      (FC03)
//
//  Workflow de configuration :
//    1. Déclarer les variables dans user/program.cpp sur la RPi
//       (VAR_INPUT pour les entrées terrain, VAR_OUTPUT pour les sorties)
//    2. Lancer la RPi — les adresses Modbus s'affichent dans le dashboard
//       http://<rpi-ip>:8080  ou dans la console au démarrage
//    3. Remplir ce fichier en faisant correspondre GPIO ↔ adresses RPi
//    4. Flasher l'ESP32 avec PlatformIO
//
//  Exemple (program.cpp sur la RPi) :
//    VAR_INPUT (BOOL, start_button, false)   → 40001 (addr=0)
//    VAR_OUTPUT(BOOL, motor_run,    false)   → 40002 (addr=1)
//    VAR_OUTPUT(INT,  speed_rpm,    0)       → 40003 (addr=2)
//
//  io_map.h correspondant :
//    DIGITAL_INPUTS[]  = { {34, 0} }   // GPIO34 → RPi addr 0 (start_button)
//    DIGITAL_OUTPUTS[] = { {2,  1} }   // GPIO2  ← RPi addr 1 (motor_run)
//    ANALOG_OUTPUTS[]  = { {25, 2} }   // GPIO25 ← RPi addr 2 (speed_rpm)
//
//  Adresse protocole = adresse affichée − 40001
//  (40001 → 0,  40002 → 1,  40003 → 2, ...)
// ============================================================================

// ── Heartbeat ─────────────────────────────────────────────────────────────────
// L'ESP32 écrit 1 à cette adresse à chaque cycle.
// La RPi efface à chaque scan → LED reste allumée tant que l'ESP32 répond.
#define HEARTBEAT_ADDR  0   // = VAR_INPUT esp32_hb addr 0 (40001)

// ── Mode ──────────────────────────────────────────────────────────────────────
// USE_ESP32  1 : connexion WiFi + Modbus active — I/O physiques via GPIO
// USE_ESP32  0 : simulation — pas de WiFi, les entrées sont forcées via dashboard
#define USE_ESP32  1

// ── Réseau (ignoré si USE_ESP32 = 0) ─────────────────────────────────────────
#define WIFI_SSID  "Hotspot"
#define WIFI_PASS  "transformeresp32"

// IP de la RPi (ou du PC en test) sur le réseau local
// Réseau local classique : 192.168.1.x  →  `ip a` (Linux) / `ipconfig` (Windows)
// Hotspot Windows         : 192.168.137.1  (IP fixe du PC côté hotspot)
#define RPI_IP     "192.168.137.1"  // IP fixe du PC côté hotspot Windows

// Port Modbus TCP de la RPi — port standard 502
// Sur Linux : sudo make run
#define RPI_PORT    502

// ── Mapping GPIO ↔ registres Modbus de la RPi ─────────────────────────────────
// Format : { gpio_pin, rpi_modbus_addr }
// rpi_modbus_addr = adresse affichée − 40001
//   (ex : 40001 → 0,  40005 → 4)

struct IoPin { uint8_t pin; uint16_t addr; };

// Entrées terrain → VAR_INPUT de la RPi (ESP32 écrit, FC06)
constexpr IoPin DIGITAL_INPUTS[] = {
   // {13, 2},   // GPIO35 → sensor_ir  (40003) — capteur IR (HIGH = objet présent)
};

// Sorties terrain ← VAR_OUTPUT de la RPi (ESP32 lit, FC03)
constexpr IoPin DIGITAL_OUTPUTS[] = {
    {26, 3},   // GPIO26 ← motor_run  (40004) — relais moteur convoyeur
    {13, 4},   // GPIO13 ← led_start  (40005) — LED indicateur départ
};

// Entrées analogiques (ADC 12-bit 0–4095) → VAR_INPUT de la RPi
constexpr IoPin ANALOG_INPUTS[] = {
    // aucune
};

// Sorties analogiques ← VAR_OUTPUT de la RPi (valeur 0–255 → PWM LEDC)
constexpr IoPin ANALOG_OUTPUTS[] = {
    // aucune
};

// Nombre d'éléments dans chaque tableau (ne pas modifier)
constexpr uint8_t N_DI = sizeof(DIGITAL_INPUTS)  / sizeof(IoPin);
constexpr uint8_t N_DO = sizeof(DIGITAL_OUTPUTS) / sizeof(IoPin);
constexpr uint8_t N_AI = sizeof(ANALOG_INPUTS)   / sizeof(IoPin);
constexpr uint8_t N_AO = sizeof(ANALOG_OUTPUTS)  / sizeof(IoPin);
