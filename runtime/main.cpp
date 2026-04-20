// ============================================================================
//  MyPLC Runtime Harness  —  do not modify this file.
//
//  Services started automatically at boot (no user code required):
//    • Web dashboard  → http://localhost:8080
//    • Modbus TCP     → 0.0.0.0:502   (FC03 / FC06 / FC16)
//
//  All VAR_INPUT / VAR_OUTPUT / VAR_MEM variables are mapped to Modbus
//  holding registers automatically, in declaration order.
//  The address table is printed at startup.
//
//  On Linux/RPi, port 502 requires root:
//    sudo make run
// ============================================================================
#include <iostream>
#include <thread>
#include <cstdlib>
#include "plc/myplc.h"
#include "sim/server.h"
#include "modbus/server.h"

using namespace std::chrono_literals;

// Defined in user/program.cpp
extern void INIT();
extern void LOOP();

int main() {
    // ── Read optional environment overrides ───────────────────────────────────
    const char* env_mb = std::getenv("MODBUS_PORT");
    uint16_t mb_port   = env_mb ? static_cast<uint16_t>(std::atoi(env_mb)) : 502;

    const char* env_http = std::getenv("HTTP_PORT");
    uint16_t http_port   = env_http ? static_cast<uint16_t>(std::atoi(env_http)) : 8080;

    // ── Banner ────────────────────────────────────────────────────────────────
    std::cout
        << "\n============================================\n"
        << "  MyPLC Runtime  |  IEC 61131-3\n"
        << "============================================\n"
        << "  Web dashboard : http://localhost:" << http_port << "\n"
        << "  Modbus TCP    : localhost:" << mb_port
        << " (FC03 read / FC06 FC16 write)\n"
        << "============================================\n";

    // ── Start services (background threads) ───────────────────────────────────
    myplc::sim::start_server(http_port);

    // Modbus server — starts automatically, prints holding register map
    myplc::ModbusTcpServer mb_server;
    mb_server.start(mb_port);

    // ── PLC programme ─────────────────────────────────────────────────────────
    INIT();
    std::cout << "[plc] Scan loop started  (cycle = 10 ms)\n\n";

    while (true) {
        LOOP();
        std::this_thread::sleep_for(10ms);
    }
}
