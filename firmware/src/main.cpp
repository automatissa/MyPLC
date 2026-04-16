// ============================================================================
//  MyPLC Firmware — ESP32 Remote I/O Board  (NE PAS MODIFIER)
//  Éditer uniquement : src/io_map.h
//
//  USE_ESP32 = 1 :  WiFi + Modbus actifs — I/O physiques reliées à la RPi
//  USE_ESP32 = 0 :  Simulation — pas de WiFi, GPIO lus/écrits localement
//                   La RPi tourne seule, les entrées se forcent via dashboard
//
//  Comportement réseau (USE_ESP32 = 1) :
//    • setup() démarre la connexion WiFi sans bloquer
//    • loop() retente automatiquement toutes les 5 s si WiFi perdu
//    • Modbus se reconnecte automatiquement dès que WiFi est disponible
//    → Flasher l'ESP32 AVANT d'activer le hotspot fonctionne parfaitement
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include "io_map.h"
#include "mb_client.h"

// ── État WiFi + Modbus ────────────────────────────────────────────────────────
#if USE_ESP32
static bool          _wifi_ok       = false;
static bool          _mb_ok         = false;
static unsigned long _last_wifi_try = 0;
static unsigned long _last_mb_print = 0;
static const unsigned long WIFI_RETRY_MS = 5000;

static void _wifi_try() {
    if (millis() - _last_wifi_try < WIFI_RETRY_MS) return;
    _last_wifi_try = millis();
    Serial.printf("[wifi] Connexion à %s...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

static bool _wifi_check() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!_wifi_ok) {
            _wifi_ok = true;
            Serial.printf("[wifi] Connecté — IP locale : %s\n",
                          WiFi.localIP().toString().c_str());
            Serial.printf("[modbus] Connexion à la RPi : %s:%d\n", RPI_IP, RPI_PORT);
        }
        return true;
    }
    if (_wifi_ok) {
        _wifi_ok = false;
        Serial.println("[wifi] Connexion perdue — retry en cours...");
    }
    _wifi_try();
    return false;
}
#endif  // USE_ESP32

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(300);

    Serial.println("============================================");
    Serial.println("  MyPLC — ESP32 Remote I/O Board");
#if USE_ESP32
    Serial.println("  Mode : Modbus TCP Client (I/O physiques)");
#else
    Serial.println("  Mode : Simulation (pas de réseau)");
#endif
    Serial.println("============================================");

    // ── LED intégrée — indicateur de connexion Modbus ────────────────────────
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);  // éteinte jusqu'à connexion Modbus confirmée

    // ── Configurer les GPIO ───────────────────────────────────────────────────
    for (uint8_t i = 0; i < N_DI; ++i)
        pinMode(DIGITAL_INPUTS[i].pin, INPUT);

    for (uint8_t i = 0; i < N_DO; ++i) {
        pinMode(DIGITAL_OUTPUTS[i].pin, OUTPUT);
        digitalWrite(DIGITAL_OUTPUTS[i].pin, LOW);
    }

    for (uint8_t i = 0; i < N_AO; ++i) {
        ledcSetup(i, 1000, 8);
        ledcAttachPin(ANALOG_OUTPUTS[i].pin, i);
        ledcWrite(i, 0);
    }

    // ── Afficher la table de correspondance GPIO ↔ registres RPi ─────────────
    Serial.println();
    Serial.println("  GPIO  Dir   RPi Modbus Addr");
    Serial.println("  ----  ---   ---------------");
    for (uint8_t i = 0; i < N_DI; ++i)
        Serial.printf("  %-5d DI    %d\n", DIGITAL_INPUTS[i].pin,  DIGITAL_INPUTS[i].addr);
    for (uint8_t i = 0; i < N_DO; ++i)
        Serial.printf("  %-5d DO    %d\n", DIGITAL_OUTPUTS[i].pin, DIGITAL_OUTPUTS[i].addr);
    for (uint8_t i = 0; i < N_AI; ++i)
        Serial.printf("  %-5d AI    %d  (ADC 0-4095)\n", ANALOG_INPUTS[i].pin,  ANALOG_INPUTS[i].addr);
    for (uint8_t i = 0; i < N_AO; ++i)
        Serial.printf("  %-5d AO    %d  (PWM 0-255)\n",  ANALOG_OUTPUTS[i].pin, ANALOG_OUTPUTS[i].addr);
    Serial.println();

#if USE_ESP32
    // Démarrer WiFi — non-bloquant : la connexion se finalise dans loop()
    WiFi.mode(WIFI_STA);
    _wifi_try();
    Serial.println("[wifi] Connexion en cours (retry automatique si non disponible)...");
#else
    Serial.println("[sim] Mode simulation — GPIO actifs, réseau désactivé");
#endif
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {

#if USE_ESP32
    // ── Vérifier WiFi — retry automatique si perdu ────────────────────────────
    if (!_wifi_check()) {
        delay(20);
        return;  // pas de Modbus sans WiFi, mais les GPIO restent configurés
    }

    // ── 1. Heartbeat — prouve que l'ESP32 est connecté et Modbus actif ─────────
    bool hb = mb_write_reg(HEARTBEAT_ADDR, 1u);
    digitalWrite(2, hb ? HIGH : LOW);  // LED on = Modbus OK, off = déconnecté

    if (hb != _mb_ok || millis() - _last_mb_print > 5000) {
        _mb_ok = hb;
        _last_mb_print = millis();
        if (hb)
            Serial.printf("[modbus] OK — RPi %s:%d accessible\n", RPI_IP, RPI_PORT);
        else
            Serial.printf("[modbus] ERREUR — impossible de joindre %s:%d\n", RPI_IP, RPI_PORT);
    }
    if (!hb) { delay(10); return; }  // Modbus KO : inutile de continuer

    // ── 2. Entrées terrain → VAR_INPUT de la RPi (FC06 write) ────────────────
    for (uint8_t i = 0; i < N_DI; ++i)
        mb_write_reg(DIGITAL_INPUTS[i].addr, digitalRead(DIGITAL_INPUTS[i].pin) ? 1u : 0u);

    for (uint8_t i = 0; i < N_AI; ++i)
        mb_write_reg(ANALOG_INPUTS[i].addr, (uint16_t)analogRead(ANALOG_INPUTS[i].pin));

    // ── 2. VAR_OUTPUT de la RPi → sorties terrain (FC03 read) ────────────────
    for (uint8_t i = 0; i < N_DO; ++i) {
        uint16_t val = 0;
        if (mb_read_reg(DIGITAL_OUTPUTS[i].addr, val))
            digitalWrite(DIGITAL_OUTPUTS[i].pin, val ? HIGH : LOW);
    }

    for (uint8_t i = 0; i < N_AO; ++i) {
        uint16_t val = 0;
        if (mb_read_reg(ANALOG_OUTPUTS[i].addr, val))
            ledcWrite(i, (uint8_t)(val & 0xFF));
    }

#else
    // ── Mode simulation : GPIO lus/écrits localement, pas de réseau ──────────
    // Les entrées sont affichées sur le port série pour debug.
    for (uint8_t i = 0; i < N_DI; ++i)
        Serial.printf("[sim] DI[%d] gpio%d = %d\n", i,
                      DIGITAL_INPUTS[i].pin, digitalRead(DIGITAL_INPUTS[i].pin));

    for (uint8_t i = 0; i < N_AI; ++i)
        Serial.printf("[sim] AI[%d] gpio%d = %d\n", i,
                      ANALOG_INPUTS[i].pin, analogRead(ANALOG_INPUTS[i].pin));
    delay(1000);  // ralentir l'affichage en mode simulation
    return;
#endif

    delay(10);  // 100 Hz
}
