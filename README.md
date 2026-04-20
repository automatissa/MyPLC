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
- **VAR_INPUT / VAR_OUTPUT / VAR_MEM** — mirrors IEC 61131-3 variable sections exactly
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
| Web dashboard | http://localhost:8080 |
| Modbus TCP server | 0.0.0.0:502 (toutes interfaces) |

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
// ── Heartbeat ESP32 (VAR_INPUT — ESP32 writes 1 every cycle) ─────────────────
VAR_INPUT(BOOL, esp32_hb,   false)  // 40001  addr 0 — connection watchdog

// ── Operator memory — writable from dashboard / SCADA ────────────────────────
VAR_MEM  (BOOL, start_btn,  false)  // 40002  addr 1 — start command (HMI)

// ── Field inputs (ESP32 → RPi via FC06) ──────────────────────────────────────
VAR_INPUT(BOOL, sensor_ir,  false)  // 40003  addr 2 — IR sensor

// ── Field outputs (RPi → ESP32 via FC03) ─────────────────────────────────────
VAR_OUTPUT(BOOL, motor_run, false)  // 40004  addr 3 — conveyor motor
VAR_OUTPUT(BOOL, led_start, false)  // 40005  addr 4 — start indicator LED

void INIT() {}

void LOOP() {
    motor_run = esp32_hb && start_btn && !sensor_ir;
    led_start = start_btn;
    esp32_hb  = false;  // watchdog — cleared each scan, ESP32 must re-write
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
┌────────────┬─────────┬──────┬────────────────┬───────┬───────────┐
│ Variable   │ Dir     │ Type │ Modbus Address │ Value │ Write     │
├────────────┼─────────┼──────┼────────────────┼───────┼───────────┤
│ esp32_hb   │ INPUT   │ BOOL │ 40001          │ TRUE  │ [  ] Set  │
│ start_btn  │ MEM     │ BOOL │ 40002          │ FALSE │ [  ] Set  │
│ sensor_ir  │ INPUT   │ BOOL │ 40003          │ FALSE │ [  ] Set  │
│ motor_run  │ OUTPUT  │ BOOL │ 40004          │ FALSE │ —         │
│ led_start  │ OUTPUT  │ BOOL │ 40005          │ FALSE │ —         │
└────────────┴─────────┴──────┴────────────────┴───────┴───────────┘
```

- Green `INPUT` badge — physical sensor input, writable from dashboard and Modbus
- Blue `OUTPUT` badge — physical actuator output, read-only (computed by the PLC)
- Purple `MEM` badge — internal memory, writable from dashboard and Modbus
- **Modbus Address** column — always visible, configure your SCADA from here
- Auto-refresh every 100 ms

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
VAR_INPUT (BOOL, esp32_hb,   false)  // 40001 — heartbeat: ESP32 writes 1 every cycle
VAR_MEM   (BOOL, start_btn,  false)  // 40002 — operator command (dashboard / SCADA)
VAR_INPUT (BOOL, sensor_ir,  false)  // 40003 — field input: IR sensor via ESP32
VAR_OUTPUT(BOOL, motor_run,  false)  // 40004 — field output: conveyor → ESP32 GPIO
VAR_OUTPUT(BOOL, led_start,  false)  // 40005 — field output: indicator LED → ESP32 GPIO
```

The **heartbeat pattern** (`esp32_hb`) is the recommended way to detect ESP32 connectivity:
- ESP32 writes `1` every cycle (10 ms) via FC06
- PLC clears it at end of LOOP — if ESP32 drops, it stays `false` within one scan
- Gate any safety-critical output behind `esp32_hb`: `motor_run = esp32_hb && ...`

Run `make run` and open the dashboard to confirm the addresses.

### Step 2 — Configure the ESP32 (one time)

Edit `firmware/src/io_map.h` — match GPIO pins to RPi register addresses:

```cpp
#define WIFI_SSID  "YourNetwork"
#define WIFI_PASS  "YourPassword"
#define RPI_IP     "192.168.137.1"  // Windows hotspot: 192.168.137.1 / RPi: its IP
#define RPI_PORT    502
#define HEARTBEAT_ADDR  0           // must match esp32_hb addr in program.cpp

// {GPIO pin, RPi register address}  (address = 4xxxx − 40001)
constexpr IoPin DIGITAL_INPUTS[]  = { {34, 2} };  // GPIO34 → sensor_ir  (addr 2)
constexpr IoPin DIGITAL_OUTPUTS[] = { {4,  3},    // GPIO4  ← motor_run  (addr 3)
                                      {13, 4} };  // GPIO13 ← led_start  (addr 4)
```

> The built-in LED (GPIO2) lights up automatically when Modbus is working — no config needed.

### Step 3 — Flash

```bash
make flash
```

The ESP32 connects to the RPi automatically and reconnects on WiFi drops or RPi reboots.

---

## Make Commands

```bash
make              # compile runtime.exe (auto-kills previous instance on Windows)
make run          # compile + start runtime (non-blocking — terminal stays free)
make flash        # flash the ESP32 firmware via PlatformIO
make clean        # remove obj/ and runtime.exe

make sample S=01_TON        # build and run a standalone sample
make samples                # list all available samples
```

Typical session:
```bash
make run          # start the RPi runtime (kills previous instance automatically)
make flash        # flash the ESP32 — same terminal, no need to open another
```

> **Windows**: `make run` must be run from the **MSYS2 UCRT64** terminal, not PowerShell. `make flash` works from anywhere since it only calls `pio`.

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
