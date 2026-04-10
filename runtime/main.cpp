// ============================================================================
//  MyPLC Runtime Harness  —  do not modify this file.
//
//  This file wires together:
//    1. The web simulator dashboard  (HTTP server on HMI_HTTP_PORT)
//    2. The Modbus TCP server        (Modbus server on MODBUS_PORT)
//    3. Your PLC program defined in user/program.cpp
//
//  Your program provides two hooks:
//    void INIT() — called once before the scan loop
//    void LOOP() — called every scan cycle
// ============================================================================
#include <iostream>
#include <thread>
#include <chrono>
#include "plc/myplc.h"
#include "sim/server.h"
#include "sim/modbus_server.h"
#include "config/network.h"

using namespace std::chrono_literals;

// Defined in user/program.cpp
extern void INIT();
extern void LOOP();

int main() {
    std::cout
        << "============================================\n"
        << "  MyPLC Runtime  |  IEC 61131-3 Simulator\n"
        << "============================================\n";

    // Start web dashboard (background thread)
    myplc::sim::start_server(HMI_HTTP_PORT);

    // Start Modbus TCP server (background thread)
    myplc::sim::start_modbus_server(MODBUS_PORT);

    // One-time initialisation
    INIT();
    std::cout << "[plc] Scan loop started  (cycle = " << SCAN_CYCLE_MS << " ms)\n\n";

    while (true) {
        LOOP();
        std::this_thread::sleep_for(std::chrono::milliseconds(SCAN_CYCLE_MS));
    }
}
