#pragma once
// ============================================================================
//  Modbus TCP Server — ESP32 / Arduino (WiFiServer-based)
//
//  Exposes all PLC_VAR variables as Modbus holding registers.
//  Supported function codes:
//    FC03 — Read Holding Registers
//    FC06 — Write Single Register
//    FC16 — Write Multiple Registers
//
//  Call mb_server_start() once in setup(), then mb_server_poll() every loop().
// ============================================================================

#include <WiFi.h>
#include "sim/registry.h"

namespace myplc::esp32 {

// Internal state
static WiFiServer* _mb_srv = nullptr;
static WiFiClient  _mb_client;

// ── Helpers ───────────────────────────────────────────────────────────────────
static bool mb_recv(uint8_t* buf, size_t n) {
    unsigned long t = millis();
    size_t got = 0;
    while (got < n) {
        if (!_mb_client.connected()) return false;
        if (_mb_client.available()) {
            buf[got++] = static_cast<uint8_t>(_mb_client.read());
        } else if (millis() - t > 1000) {
            return false;  // timeout
        }
    }
    return true;
}

static void mb_send(const uint8_t* buf, size_t n) {
    _mb_client.write(buf, n);
}

static void mb_send_exception(const uint8_t* hdr, uint8_t fc, uint8_t code) {
    uint8_t resp[9];
    resp[0] = hdr[0]; resp[1] = hdr[1];
    resp[2] = 0;      resp[3] = 0;
    resp[4] = 0;      resp[5] = 3;
    resp[6] = hdr[6];
    resp[7] = static_cast<uint8_t>(fc | 0x80u);
    resp[8] = code;
    mb_send(resp, 9);
}

// ── Public API ────────────────────────────────────────────────────────────────
inline void mb_server_start(uint16_t port = 502) {
    _mb_srv = new WiFiServer(port);
    _mb_srv->begin();
    Serial.printf("[modbus] Modbus TCP server on port %d\n", port);
}

// Call from Arduino loop() — handles one request per call (non-blocking poll)
inline void mb_server_poll() {
    // Accept new client if none connected
    if (!_mb_client || !_mb_client.connected()) {
        _mb_client = _mb_srv->available();
        if (!_mb_client) return;
        Serial.println("[modbus] Client connected");
    }

    // Check if data available
    if (_mb_client.available() < 7) return;

    uint8_t hdr[7];  // 6-byte MBAP + unit ID
    if (!mb_recv(hdr, 7)) { _mb_client.stop(); return; }

    uint16_t pdu_len = static_cast<uint16_t>((hdr[4] << 8) | hdr[5]);
    if (pdu_len < 2 || pdu_len > 256) { _mb_client.stop(); return; }

    uint8_t pdu[256];
    size_t  data_len = pdu_len - 1;  // unit ID already consumed
    if (!mb_recv(pdu, data_len)) { _mb_client.stop(); return; }

    uint8_t  fc   = pdu[0];
    uint16_t addr = static_cast<uint16_t>((pdu[1] << 8) | pdu[2]);
    uint16_t qty  = static_cast<uint16_t>((pdu[3] << 8) | pdu[4]);

    if (fc == 0x03) {
        // FC03 — Read Holding Registers
        if (qty < 1 || qty > 125) { mb_send_exception(hdr, fc, 0x03); return; }
        uint16_t regs[125] = {};
        myplc::sim::Registry::get().fill_response_regs(addr, qty, regs);

        uint8_t resp[9 + 125*2];
        resp[0] = hdr[0]; resp[1] = hdr[1];
        resp[2] = 0;      resp[3] = 0;
        uint16_t rlen = static_cast<uint16_t>(3 + qty * 2);
        resp[4] = static_cast<uint8_t>(rlen >> 8);
        resp[5] = static_cast<uint8_t>(rlen & 0xFF);
        resp[6] = hdr[6];
        resp[7] = fc;
        resp[8] = static_cast<uint8_t>(qty * 2);
        for (uint16_t i = 0; i < qty; ++i) {
            resp[9  + i*2] = static_cast<uint8_t>(regs[i] >> 8);
            resp[10 + i*2] = static_cast<uint8_t>(regs[i] & 0xFF);
        }
        mb_send(resp, 9 + qty * 2);

    } else if (fc == 0x06) {
        // FC06 — Write Single Register
        uint16_t value = static_cast<uint16_t>((pdu[3] << 8) | pdu[4]);
        myplc::sim::Registry::get().apply_write_regs(addr, 1, &value);
        mb_send(hdr, 7);
        mb_send(pdu, data_len);

    } else if (fc == 0x10) {
        // FC16 — Write Multiple Registers
        if (data_len < 6) { mb_send_exception(hdr, fc, 0x03); return; }
        uint8_t byte_count = pdu[5];
        if (byte_count != qty * 2) { mb_send_exception(hdr, fc, 0x03); return; }
        uint16_t regs[125] = {};
        for (uint16_t i = 0; i < qty && i < 125; ++i) {
            regs[i] = static_cast<uint16_t>((pdu[6 + i*2] << 8) | pdu[7 + i*2]);
        }
        myplc::sim::Registry::get().apply_write_regs(addr, qty, regs);

        uint8_t resp[12];
        resp[0] = hdr[0]; resp[1] = hdr[1];
        resp[2] = 0;      resp[3] = 0;
        resp[4] = 0;      resp[5] = 6;
        resp[6] = hdr[6];
        resp[7] = fc;
        resp[8] = pdu[1]; resp[9]  = pdu[2];
        resp[10]= pdu[3]; resp[11] = pdu[4];
        mb_send(resp, 12);

    } else {
        mb_send_exception(hdr, fc, 0x01);  // Illegal Function
    }
}

}  // namespace myplc::esp32
