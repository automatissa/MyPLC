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
- **Web simulator** — run your program locally and watch all variables live in a browser dashboard
- **Scalable** — one header/source pair per Function Block; add new FBs in minutes

---

## Requirements

| Tool | Version |
|------|---------|
| C++ compiler | GCC ≥ 9 / Clang ≥ 10 / MSVC 2019+ |
| C++ standard | C++17 |
| Make | GNU Make |
| Browser | Any (for the dashboard) |

On **Windows**: use [MSYS2 UCRT64](https://www.msys2.org/) — `pacman -S mingw-w64-ucrt-x86_64-gcc make`

---

## Quick Start

```bash
git clone https://github.com/automatissa/myplc.git
cd myplc
make          # compile
make run      # run + open http://localhost:8080 in your browser
```

---

## Writing Your Program

**You only edit one file: `user/program.cpp`**

The file has three sections that mirror a Structured Text program:

```
┌─────────────────────────────────┐     ┌──────────────────────────────────┐
│  Structured Text                │     │  MyPLC C++                       │
├─────────────────────────────────┤     ├──────────────────────────────────┤
│  PROGRAM Main                   │     │  // user/program.cpp             │
│    VAR                          │     │                                  │
│      sensor  : BOOL := FALSE;   │ ──► │  PLC_VAR(BOOL, sensor,  false)   │
│      motor   : BOOL := FALSE;   │     │  PLC_VAR(BOOL, motor,   false)   │
│      count   : INT  := 0;       │     │  PLC_VAR(INT,  count,   0)       │
│      delay   : TON;             │     │  myplc::TON delay;               │
│    END_VAR                      │     │                                  │
│                                 │     │  void INIT() {                   │
│    (* init *)                   │ ──► │      delay.PT(T(5s));            │
│    delay.PT := T#5s;            │     │  }                               │
│                                 │     │                                  │
│    delay(IN := sensor,          │     │  void LOOP() {                   │
│           PT := T#5s);          │ ──► │      delay(sensor, T(5s));       │
│    motor := delay.Q;            │     │      motor = delay.Q();          │
│    count := count + 1;          │     │      count++;                    │
│  END_PROGRAM                    │     │  }                               │
└─────────────────────────────────┘     └──────────────────────────────────┘
```

### PLC_VAR macro

`PLC_VAR(TYPE, name, initial_value)` does two things at once:
1. Declares the global variable (like `VAR ... END_VAR`)
2. Registers it with the web dashboard so you can read and write it live

```cpp
PLC_VAR(BOOL, start_button, false)   // writable input  — toggle in dashboard
PLC_VAR(BOOL, motor_run,    false)   // readable output — visible in dashboard
PLC_VAR(INT,  cycle_time_ms, 0)      // any scalar value you want to watch
```

### Supported types for PLC_VAR

| Type | ST equivalent | C++ type |
|------|--------------|----------|
| `BOOL` | `BOOL` | `bool` |
| `INT` | `INT` | `int16_t` |
| `DINT` | `DINT` | `int32_t` |
| `REAL` | `REAL` | `float` |
| `LREAL` | `LREAL` | `double` |

### Function Block instances

Declare FB instances as plain globals (they are too complex to register):

```cpp
myplc::TON  delay;
myplc::CTU  part_counter;
myplc::SR   motor_latch;
```

---

## Available Function Blocks

| Category | Name | Description |
|----------|------|-------------|
| **Timers** | `TON` | On-delay: Q goes TRUE after IN held TRUE for PT |
| | `TOF` | Off-delay: Q stays TRUE for PT after IN goes FALSE |
| | `TP` | Pulse: Q goes TRUE for exactly PT on rising edge (non-retriggerable) |
| **Edge detectors** | `R_TRIG` | Q=TRUE for one scan on rising edge (FALSE→TRUE) |
| | `F_TRIG` | Q=TRUE for one scan on falling edge (TRUE→FALSE) |
| **Counters** | `CTU` | Count Up: CV++ on rising CU, Q when CV≥PV |
| | `CTD` | Count Down: CV-- on rising CD, Q when CV≤0 |
| | `CTUD` | Count Up/Down: QU when CV≥PV, QD when CV≤0 |
| **Bistables** | `SR` | Set-dominant latch: Q1 = S1 OR (Q1 AND NOT R) |
| | `RS` | Reset-dominant latch: Q1 = (S OR Q1) AND NOT R1 |

### Call syntax

Every FB supports the compact ST call style:

```cpp
// Timers
delay(sensor, T(3s));           // IN, PT
motor_off_delay(run, T(500ms));

// Counters
parts(cu_pulse, reset, 10);     // CU, R, PV
stock(cd_pulse, load, 10);      // CD, LD, PV
bidirectional(cu, cd, reset, load, 10);  // CTUD

// Edge detectors
rising(button);
falling(button);

// Bistables
latch(set_btn, reset_btn);

// Read outputs
if (delay.Q())     motor = true;
int ms = delay.ET();            // elapsed ms
int cv = parts.CV();            // current count
```

---

## Web Simulator Dashboard

After `make run`, open **http://localhost:8080** in any browser.

The dashboard shows a live table of all `PLC_VAR` variables:

```
┌─────────────────┬────────┬────────────────┬──────────────────────┐
│ Variable        │ Type   │ Value          │ Write                │
├─────────────────┼────────┼────────────────┼──────────────────────┤
│ start_button    │ BOOL   │ ▐▐ TRUE        │ [true   ] [Set]      │
│ motor_run       │ BOOL   │ □□ FALSE       │ [false  ] [Set]      │
│ cycle_time_ms   │ INT    │ 2340           │ [0      ] [Set]      │
└─────────────────┴────────┴────────────────┴──────────────────────┘
```

- Table **auto-refreshes every 500 ms**
- Type a value in the Write field and click **Set** to inject it into the running program
- This lets you simulate button presses, sensor changes, setpoint adjustments — without any hardware

---

## Make Commands

```bash
make              # compile runtime.exe
make run          # compile + run (then open http://localhost:8080)
make clean        # remove obj/ and runtime.exe

make sample S=01_TON        # build and run a standalone sample
make samples                # list all available samples
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
│   ├── registry.h          ← PLC_VAR macro
│   └── server.cpp          ← HTTP server + embedded dashboard
│
├── runtime/main.cpp        ← PLC harness (never edit)
│
├── user/
│   └── program.cpp         ← ★ YOUR CODE GOES HERE ★
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
4. Done — it is now available to the user and all samples

---

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE)
