// ============================================================================
//  MyPLC Runtime Harness  —  do not modify this file.
//
//  This file wires together:
//    1. The web simulator dashboard (starts an HTTP server on port 8080)
//    2. Your PLC program defined in user/program.cpp
//
//  Your program provides two hooks:
//    void INIT() — called once before the scan loop
//    void LOOP() — called every scan cycle (default: 10 ms)
// ============================================================================
#include <iostream>
#include <thread>
#include "plc/myplc.h"
#include "sim/server.h"

using namespace std::chrono_literals;

// Defined in user/program.cpp
extern void INIT();
extern void LOOP();

int main() {
    std::cout
        << "============================================\n"
        << "  MyPLC Runtime  |  IEC 61131-3 Simulator\n"
        << "============================================\n";

    // Start the web dashboard in a background thread
    myplc::sim::start_server(8080);

    // One-time initialisation (mirrors the program's init section in ST)
    INIT();
    std::cout << "[plc] Scan loop started  (cycle = 10 ms)\n\n";

    // Cyclic scan loop
    while (true) {
        LOOP();
        std::this_thread::sleep_for(10ms);
    }
}
