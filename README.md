# MyPLC — IEC 61131-3 Function Blocks in C++

**MyPLC** is a C++ library that reimplements the standard IEC 61131-3 Function Blocks
so that engineers familiar with Structured Text (ST) can write PLC programs in C++
with the same structure and syntax they already know.

> Created by an industrial automation engineer who believes C++ is the right
> language to bridge IT and OT — and that ST programmers deserve first-class tooling. Powered by Claude Code.

---

## Why MyPLC?

- **Familiar API** — ST call style works as-is: `myTimer(sensor, T(5s));`
- **Standard types** — `BOOL`, `INT`, `DINT`, `REAL`, `TIME` map directly to C++ primitives
- **VAR_INPUT / VAR_OUTPUT / VAR** — mirrors IEC 61131-3 variable sections exactly
- **Web dashboard** — live variable table with Modbus addresses, direction badges, write field
- **Modbus TCP server** — all variables exposed as holding registers automatically, no code needed
- **ESP32 remote I/O** — dumb I/O board connects to the RPi as a Modbus client; zero extra code in your PLC program

---

## Requirements

| Tool | Version |
|------|---------|
| C++ compiler | GCC ≥ 9 / Clang ≥ 10 |
| C++ standard | C++17 |
| Make | GNU Make |
| Browser | Any (for the dashboard) |

On **Windows**: use [MSYS2 UCRT64](https://www.msys2.org/) — `pacman -S mingw-w64-ucrt-x86_64-gcc make`

For **ESP32 firmware**: [PlatformIO](https://platformio.org/)

---

## Quick Start

### RPi / PC

```bash
git clone https://github.com/automatissa/myplc.git
cd myplc
make run          # compile + lance le runtime
```

> Linux / RPi — port 502 nécessite root : `sudo make run`

Ouvrir **http://localhost:8080** — le dashboard s'affiche avec toutes les variables, leurs adresses Modbus et les champs d'écriture.

`make run` est non-bloquant : sous Windows le runtime s'ouvre dans une nouvelle fenêtre, sous Linux il passe en arrière-plan. Le terminal reste libre immédiatement.

### ESP32

```bash
# 1. Éditer firmware/src/io_map.h
#    → WIFI_SSID, WIFI_PASS, RPI_IP, mapping GPIO ↔ adresses Modbus

make flash        # compile + flash via PlatformIO
```

Même terminal — pas besoin d'ouvrir PlatformIO manuellement.

### Workflow complet

```bash
make run          # RPi : runtime démarré, dashboard accessible
make flash        # ESP32 : flashé dans la foulée
# → ouvrir http://localhost:8080 pour voir les variables en temps réel
# → ouvrir le Serial Monitor (115200) pour voir la connexion ESP32
```

---

## Services démarrés automatiquement

Au lancement, deux services démarrent en arrière-plan — aucun code requis :

| Service | Adresse |
|---------|---------|
| Web dashboard | http://0.0.0.0:8080 |
| Modbus TCP server | 0.0.0.0:502 |

La table des registres s'affiche au démarrage et dans le dashboard :

```
  Holding Registers — FC03 read / FC06 FC16 write
  Variable                 Type       Addr  4xxxx  Regs
  ------------------------ --------  -----  -------  ----
  start_button             BOOL          0  40001  1
  motor_run                BOOL          1  40002  1
  cycle_time_ms            INT           2  40003  1
```

---

## Writing Your Program

**You only edit one file: `user/program.cpp`**

```cpp
// ── Variable declarations ─────────────────────────────────────────────────────
VAR_INPUT (BOOL, start_button,  false)  // 40001 — ESP32 writes sensor value here
VAR_OUTPUT(BOOL, motor_run,     false)  // 40002 — ESP32 reads actuator command here
VAR_OUTPUT(INT,  cycle_time_ms, 0)      // 40003 — computed by PLC, read-only

myplc::TON delay;  // Function block — declared as plain global

// ── Initialisation ───────────────────────────────────────────────────────────
void INIT() {
    delay.PT(T(5s));
}

// ── Main scan loop (called every 10 ms) ─────────────────────────────────────
void LOOP() {
    delay(start_button, T(5s));
    motor_run     = delay.Q();
    cycle_time_ms = delay.ET();
}
```

### Variable macros

| Macro | Meaning | Dashboard | Modbus write |
|-------|---------|-----------|-------------|
| `VAR_INPUT(TYPE, name, init)` | Physical input — ESP32 → RPi | Write field active | Yes (FC06/FC16) |
| `VAR_OUTPUT(TYPE, name, init)` | Physical output — RPi → ESP32 | Read-only | No |
| `VAR_MEM(TYPE, name, init)` | Internal memory — setpoints, state | Write field active | Yes (FC06/FC16) |

### Supported types

| Type | ST equivalent | C++ type | Modbus registers |
|------|--------------|----------|-----------------|
| `BOOL` | `BOOL` | `bool` | 1 |
| `INT` | `INT` | `int16_t` | 1 |
| `DINT` | `DINT` | `int32_t` | 2 |
| `REAL` | `REAL` | `float` | 2 |
| `LREAL` | `LREAL` | `double` | 4 |
| `TIME` | `TIME` | `std::chrono::milliseconds` | 2 |

---

## Web Simulator Dashboard

After `make run`, open **http://localhost:8080**.

```
┌───────────────┬─────────┬──────┬────────────────┬───────┬───────────┐
│ Variable      │ Dir     │ Type │ Modbus Address │ Value │ Write     │
├───────────────┼─────────┼──────┼────────────────┼───────┼───────────┤
│ start_button  │ INPUT   │ BOOL │ 40001          │ FALSE │ [  ] Set  │
│ motor_run     │ OUTPUT  │ BOOL │ 40002          │ FALSE │ —         │
│ cycle_time_ms │ OUTPUT  │ INT  │ 40003          │ 2340  │ —         │
│ speed_setpt   │ MEM     │ INT  │ 40004          │ 1500  │ [  ] Set  │
└───────────────┴─────────┴──────┴────────────────┴───────┴───────────┘
```

- Green `INPUT` badge — physical sensor input, writable from dashboard and Modbus
- Blue `OUTPUT` badge — physical actuator output, read-only (computed by the PLC)
- Purple `MEM` badge — internal memory, writable from dashboard and Modbus
- **Modbus Address** column — always visible, configure your SCADA from here
- Auto-refresh every 500 ms

---

## Available Function Blocks

| Category | Name | Description |
|----------|------|-------------|
| **Timers** | `TON` | On-delay: Q goes TRUE after IN held TRUE for PT |
| | `TOF` | Off-delay: Q stays TRUE for PT after IN goes FALSE |
| | `TP` | Pulse: Q goes TRUE for exactly PT on rising edge |
| **Edge detectors** | `R_TRIG` | Q=TRUE for one scan on rising edge |
| | `F_TRIG` | Q=TRUE for one scan on falling edge |
| **Counters** | `CTU` | Count Up: CV++ on rising CU, Q when CV≥PV |
| | `CTD` | Count Down: CV-- on rising CD, Q when CV≤0 |
| | `CTUD` | Count Up/Down: QU when CV≥PV, QD when CV≤0 |
| **Bistables** | `SR` | Set-dominant latch |
| | `RS` | Reset-dominant latch |

### Call syntax

```cpp
delay(sensor, T(3s));               // TON/TOF/TP — IN, PT
parts(cu_pulse, reset, 10);         // CTU — CU, R, PV
stock(cd_pulse, load, 10);          // CTD — CD, LD, PV
bidi(cu, cd, reset, load, 10);      // CTUD
rising(button);                     // R_TRIG
falling(button);                    // F_TRIG
latch(set_btn, reset_btn);          // SR
rlatch(set_btn, reset_btn);         // RS

motor = delay.Q();     // read Q output
int ms = delay.ET();   // elapsed time in ms
int cv = parts.CV();   // current count
```

---

## ESP32 Remote I/O

The RPi runs the PLC logic. The ESP32 handles physical I/O.

```
RPi (Modbus TCP Server, port 502)
  ↑  FC06 write  GPIO inputs  → VAR_INPUT registers
  ↓  FC03 read   VAR_OUTPUT registers → GPIO outputs
ESP32 (Modbus TCP Client — dumb I/O board)
```

**Why this architecture?**
- Short circuits or wiring faults damage the ESP32 (cheap ≈ €5), not the RPi
- The ESP32 is physically close to the power/actuators; the RPi stays isolated
- The RPi's Modbus TCP server is already running — no extra code in `program.cpp`
- Any SCADA can also connect to the RPi and read/write the same registers

### Step 1 — Declare your variables (RPi)

```cpp
// user/program.cpp
VAR_INPUT (BOOL, start_button, false)  // 40001 — physical input:  ESP32 GPIO34 → RPi
VAR_OUTPUT(BOOL, motor_run,    false)  // 40002 — physical output: RPi → ESP32 GPIO2
VAR_OUTPUT(INT,  speed_rpm,    0)      // 40003 — physical output: RPi → ESP32 GPIO25 PWM
VAR_MEM   (INT,  speed_setpoint, 1500) // 40004 — internal memory: writable setpoint
```

Run `make run` and open the dashboard to confirm the addresses.

### Step 2 — Configure the ESP32 (one time)

Edit `firmware/src/io_map.h` — match GPIO pins to RPi register addresses:

```cpp
#define WIFI_SSID  "YourNetwork"
#define WIFI_PASS  "YourPassword"
#define RPI_IP     "192.168.1.10"  // RPi IP address
#define RPI_PORT    502

// {GPIO pin, RPi register address}  (address = 4xxxx displayed − 40001)
constexpr IoPin DIGITAL_INPUTS[]  = { {34, 0} };  // GPIO34 → start_button (addr 0)
constexpr IoPin DIGITAL_OUTPUTS[] = { {2,  1} };  // GPIO2  ← motor_run    (addr 1)
constexpr IoPin ANALOG_OUTPUTS[]  = { {25, 2} };  // GPIO25 ← speed_rpm    (addr 2)
```

### Step 3 — Flash

```bash
make flash
```

The ESP32 connects to the RPi automatically and reconnects on WiFi drops or RPi reboots.

---

## Make Commands

```bash
make              # compile runtime.exe
make run          # compile + start runtime (non-blocking — terminal stays free)
make flash        # flash the ESP32 firmware via PlatformIO
make clean        # remove obj/ and runtime.exe

make sample S=01_TON        # build and run a standalone sample
make samples                # list all available samples
```

Typical session:
```bash
make run          # start the RPi runtime
make flash        # flash the ESP32 — same terminal, no need to open another
```

---

## Project Structure

```
MyPLC/
├── plc/                    ← Function Block library (never edit)
│   ├── types.h             ← IEC 61131-3 types + T() helper
│   ├── myplc.h             ← single master include
│   ├── timers/             ← TON, TOF, TP
│   ├── counters/           ← CTU, CTD, CTUD
│   ├── triggers/           ← R_TRIG, F_TRIG
│   └── bistables/          ← SR, RS
│
├── sim/                    ← Web simulator (never edit)
│   ├── registry.h          ← VAR_INPUT / VAR_OUTPUT / VAR_MEM macros + Modbus mapping
│   └── server.cpp          ← HTTP server + embedded dashboard
│
├── modbus/                 ← Modbus TCP server for RPi (never edit)
│   ├── server.h
│   └── server.cpp          ← FC03 / FC06 / FC16
│
├── firmware/               ← ESP32 dumb I/O board (PlatformIO)
│   ├── platformio.ini
│   └── src/
│       ├── io_map.h        ← ★ EDIT THIS — GPIO ↔ RPi register mapping ★
│       ├── mb_client.h     ← Modbus TCP client for ESP32 (never edit)
│       └── main.cpp        ← ESP32 setup/loop (never edit)
│
├── runtime/main.cpp        ← PLC harness — scan cycle, auto-starts services (never edit)
│
├── user/
│   └── program.cpp         ← ★ YOUR PLC LOGIC GOES HERE ★
│
├── samples/                ← standalone examples (read-only reference)
│   ├── 01_TON/
│   ├── 02_TOF/
│   ├── 03_TP/
│   ├── 04_CTU/
│   ├── 05_CTD/
│   ├── 06_CTUD/
│   ├── 07_R_TRIG_F_TRIG/
│   └── 08_SR_RS/
│
└── Makefile
```

---

## Adding a New Function Block

1. Create `plc/<category>/MYBLOCK.h` and `plc/<category>/MYBLOCK.cpp`
2. Add `#include "plc/<category>/MYBLOCK.h"` to `plc/myplc.h`
3. Add `plc/<category>/MYBLOCK.cpp` to `PLC_SRCS` in the `Makefile`

---

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE)
