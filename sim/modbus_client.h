#pragma once
#include <cstdint>
#include <string>

// ============================================================================
//  Modbus TCP Client (Raspberry Pi HMI → ESP32 PLC)
//
//  Runs a background thread that:
//    1. Polls all holding registers from the ESP32 every `poll_ms` milliseconds
//       and writes them into the local Registry (updates web dashboard).
//    2. Watches for dirty variables (written from web dashboard) and sends
//       FC06/FC16 write requests to the ESP32 so changes take effect on the PLC.
//
//  Usage in hmi_main.cpp:
//    myplc::sim::start_modbus_client("192.168.1.100", 502, 50);
// ============================================================================

namespace myplc::sim {

// Connect to ESP32 Modbus server and start polling/pushing in a background thread.
// Reconnects automatically if the connection is lost.
void start_modbus_client(const std::string& host, uint16_t port = 502, uint32_t poll_ms = 50);

}  // namespace myplc::sim
