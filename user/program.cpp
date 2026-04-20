// ============================================================================
//  MyPLC — User Program
//  Éditer uniquement ce fichier pour écrire la logique PLC.
//
//  Structure identique à IEC 61131-3 Structured Text (ST) :
//
//    ┌──────────────────────────────────────────────┐
//    │  PROGRAM Main                                │
//    │    VAR_INPUT                                 │
//    │      start_button : BOOL := FALSE;           │
//    │    END_VAR                                   │
//    │    VAR_OUTPUT                                │
//    │      motor_run    : BOOL := FALSE;           │
//    │      cycle_time   : INT  := 0;               │
//    │    END_VAR                                   │
//    │    VAR                                       │
//    │      delay        : TON;                     │
//    │    END_VAR                                   │
//    │    delay(IN := start_button, PT := T#5s);    │
//    │    motor_run  := delay.Q;                    │
//    │  END_PROGRAM                                 │
//    └──────────────────────────────────────────────┘
//
//  ─── Services démarrés automatiquement ──────────────────────────────────
//
//  Web dashboard  →  http://localhost:8080
//  Modbus TCP     →  0.0.0.0:502   (FC03 read / FC06 FC16 write)
//
//  Toutes les VAR_INPUT / VAR_OUTPUT / VAR_MEM sont exposées en holding registers.
//  La table d'adresses s'affiche au démarrage et dans le dashboard.
//  Sur Linux/RPi : sudo make run
//
//  ─── Architecture avec ESP32 Remote I/O ─────────────────────────────────
//
//  La RPi est le Modbus TCP Server.  L'ESP32 est le Modbus TCP Client.
//
//    RPi (ce fichier)                  ESP32 (firmware/src/io_map.h)
//    ─────────────────                 ──────────────────────────────
//    VAR_INPUT  start_button  ←─FC06── GPIO34 (bouton)
//    VAR_OUTPUT motor_run     ──FC03─→ GPIO2  (relais)
//
//  Workflow :
//    1. Déclarer les variables ici (VAR_INPUT / VAR_OUTPUT)
//    2. Lancer la RPi → noter les adresses dans le dashboard
//    3. Renseigner firmware/src/io_map.h (GPIO ↔ adresse RPi)
//    4. Flasher l'ESP32 avec PlatformIO
//
//  Open http://localhost:8080 after `make run` to see the dashboard.
// ============================================================================

#include "plc/myplc.h"
#include "sim/registry.h"

using namespace myplc;

// ── Variable Declarations ────────────────────────────────────────────────────
//   VAR_INPUT (TYPE, name, init)  — physical input  (ESP32 → RPi)   writable
//   VAR_OUTPUT(TYPE, name, init)  — physical output (RPi  → ESP32)  read-only
//   VAR_MEM   (TYPE, name, init)  — internal memory                  writable
//
// All variables are auto-assigned a Modbus holding register address and
// shown with their address in the web dashboard.

// ── Heartbeat ────────────────────────────────────────────────────────────────
VAR_INPUT(BOOL, esp32_hb,   false)  // 40001  addr 0 — ESP32 écrit 1 à chaque cycle

// ── Mémoire opérateur (HMI / dashboard) ──────────────────────────────────────
VAR_MEM(BOOL,   start_btn,  false)  // 40002  addr 1 — bouton départ (forcé depuis HMI)

// ── Entrées terrain (ESP32 → RPi via FC06) ───────────────────────────────────
VAR_INPUT(BOOL, sensor_ir,  false)  // 40003  addr 2 — capteur IR (true = objet détecté)

// ── Sorties terrain (RPi → ESP32 via FC03) ───────────────────────────────────
VAR_OUTPUT(BOOL, motor_run, false)  // 40004  addr 3 — moteur convoyeur
VAR_OUTPUT(BOOL, led_start, false)  // 40005  addr 4 — LED indicateur départ (GPIO13)

void INIT() {}

void LOOP() {
    // Moteur actif si ESP32 connecté, départ enclenché ET aucun objet
    motor_run = esp32_hb && start_btn && !sensor_ir;
    
    // LED allumée tant que start est actif
    led_start = start_btn;

    // Watchdog — effacé EN FIN de scan : l'ESP32 doit réécrire 1 avant le prochain
    esp32_hb = false;
}
