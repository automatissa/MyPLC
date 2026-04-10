#pragma once
#include <cstdint>

// ============================================================================
//  Modbus TCP Server (Linux / Raspberry Pi)
//
//  Starts a background thread that listens on the given port and serves
//  Modbus TCP requests.  All PLC_VAR variables are automatically mapped to
//  holding registers via the Registry.
//
//  Supported function codes:
//    FC03 — Read Holding Registers
//    FC06 — Write Single Register
//    FC16 — Write Multiple Registers
// ============================================================================

namespace myplc::sim {

// Start the Modbus TCP server in a background thread.
// Default port: 502  (requires root on Linux; use 5020 for non-root testing)
void start_modbus_server(uint16_t port = 502);

}  // namespace myplc::sim
