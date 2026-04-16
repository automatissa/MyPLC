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
//  Web dashboard  →  http://0.0.0.0:8080
//  Modbus TCP     →  0.0.0.0:502   (FC03 read / FC06 FC16 write)
//
//  Toutes les VAR_INPUT/VAR_OUTPUT sont exposées en holding registers.
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

VAR_INPUT(BOOL, esp32_connected, false)  // 40001  addr 0 — 1 si ESP32 joignable via Modbus

void INIT() {}

void LOOP() {
    //esp32_connected = false;  // watchdog : effacé chaque scan, l'ESP32 doit réécrire
}
