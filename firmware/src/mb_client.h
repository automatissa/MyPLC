#pragma once
// ============================================================================
//  mb_client.h — Client Modbus TCP pour ESP32  (NE PAS MODIFIER)
//
//  Se connecte à la RPi (Modbus TCP Server) et expose deux primitives :
//
//    mb_write_reg(addr, val)   FC06 — écrit 1 registre sur la RPi
//    mb_read_reg (addr, val)   FC03 — lit  1 registre depuis la RPi
//
//  La connexion TCP est maintenue en permanence et rétablie automatiquement
//  si la RPi redémarre ou si le réseau est coupé.
// ============================================================================

#include <WiFi.h>
#include <cstdint>
#include "io_map.h"

static WiFiClient _mbc;
static uint16_t   _mbc_tid = 0;

// ── Connexion / reconnexion automatique ───────────────────────────────────────
static bool _mbc_ensure() {
    if (_mbc.connected()) return true;
    _mbc.stop();
    return _mbc.connect(RPI_IP, RPI_PORT);
}

// ── Réception avec timeout 500 ms ─────────────────────────────────────────────
static bool _mbc_recv(uint8_t* buf, int n) {
    unsigned long t = millis();
    int got = 0;
    while (got < n) {
        if (!_mbc.connected()) return false;
        if (_mbc.available()) buf[got++] = (uint8_t)_mbc.read();
        else if (millis() - t > 500) return false;
    }
    return true;
}

// ── FC06 — Write Single Register ──────────────────────────────────────────────
// Utilisé pour pousser une entrée GPIO vers un VAR_INPUT de la RPi.
static bool mb_write_reg(uint16_t addr, uint16_t val) {
    if (!_mbc_ensure()) return false;
    uint16_t tid = _mbc_tid++;
    uint8_t req[12] = {
        (uint8_t)(tid >> 8), (uint8_t)(tid & 0xFF),
        0, 0, 0, 6,           // protocol=0, PDU length=6
        0x01,                 // unit ID
        0x06,                 // FC06
        (uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF),
        (uint8_t)(val  >> 8), (uint8_t)(val  & 0xFF)
    };
    _mbc.write(req, 12);
    uint8_t resp[12];
    if (!_mbc_recv(resp, 12) || resp[7] != 0x06) {
        _mbc.stop();
        return false;
    }
    return true;
}

// ── FC03 — Read Single Register ───────────────────────────────────────────────
// Utilisé pour lire un VAR_OUTPUT de la RPi et l'appliquer à un GPIO.
static bool mb_read_reg(uint16_t addr, uint16_t& val) {
    if (!_mbc_ensure()) return false;
    uint16_t tid = _mbc_tid++;
    uint8_t req[12] = {
        (uint8_t)(tid >> 8), (uint8_t)(tid & 0xFF),
        0, 0, 0, 6,           // protocol=0, PDU length=6
        0x01,                 // unit ID
        0x03,                 // FC03
        (uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF),
        0, 1                  // quantity = 1 register
    };
    _mbc.write(req, 12);
    uint8_t resp[11];         // MBAP(7) + FC(1) + byte_count(1) + data(2)
    if (!_mbc_recv(resp, 11) || resp[7] != 0x03) {
        _mbc.stop();
        return false;
    }
    val = (uint16_t)((resp[9] << 8) | resp[10]);
    return true;
}
