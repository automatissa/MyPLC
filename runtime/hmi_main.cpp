// ============================================================================
//  MyPLC HMI Harness  —  do not modify this file.
//
//  Runs on the Raspberry Pi in HMI mode:
//    • Connects to the ESP32 Modbus TCP server as a client
//    • Polls ESP32 PLC registers and reflects them in the local Registry
//    • Pushes web-dashboard writes back to the ESP32 via Modbus FC16
//    • Serves the web dashboard on HMI_HTTP_PORT
//
//  The PLC program (INIT/LOOP) does NOT run here — only the variable
//  declarations (PLC_VAR) run to populate the Registry with the correct
//  names, types, and Modbus addresses (must match user/program.cpp).
//
//  Build: make hmi
//  Run:   ./runtime_hmi
// ============================================================================
#include <iostream>
#include <thread>
#include <chrono>
#include "plc/myplc.h"
#include "sim/server.h"
#include "sim/modbus_client.h"
#include "config/network.h"

using namespace std::chrono_literals;

int main() {
    std::cout
        << "============================================\n"
        << "  MyPLC HMI  |  Modbus TCP Client\n"
        << "============================================\n"
        << "  ESP32 target : " << ESP32_IP << ":" << MODBUS_PORT << "\n"
        << "  Web dashboard: http://localhost:" << HMI_HTTP_PORT << "\n"
        << "============================================\n";

    // Start web dashboard (background thread)
    myplc::sim::start_server(HMI_HTTP_PORT);

    // Start Modbus client — polls ESP32 and pushes dirty vars (background thread)
    myplc::sim::start_modbus_client(ESP32_IP, MODBUS_PORT, SCAN_CYCLE_MS);

    // Run forever — the background threads handle everything
    while (true) {
        std::this_thread::sleep_for(1s);
    }
}
