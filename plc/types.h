#pragma once
#include <cstdint>

// ============================================================================
//  IEC 61131-3 Standard Data Types — cross-platform (Linux, RPi, ESP32)
// ============================================================================

using BOOL   = bool;
using BYTE   = uint8_t;
using WORD   = uint16_t;
using DWORD  = uint32_t;
using INT    = int16_t;
using UINT   = uint16_t;
using DINT   = int32_t;
using UDINT  = uint32_t;
using LINT   = int64_t;
using REAL   = float;
using LREAL  = double;

// TIME is always milliseconds (uint32_t).
// Same type on ESP32 (millis()) and Linux/RPi (steady_clock).
using TIME = uint32_t;

// ── TIME constructor helpers ──────────────────────────────────────────────
// Use instead of T#5s / T#500ms in ST.
constexpr TIME T_ms(uint32_t ms)  { return ms; }
constexpr TIME T_s (uint32_t s)   { return s   * 1000UL; }
constexpr TIME T_min(uint32_t m)  { return m   * 60000UL; }
constexpr TIME T_h (uint32_t h)   { return h   * 3600000UL; }

// ── Platform millisecond source ───────────────────────────────────────────
#ifdef ARDUINO
  // ESP32 / Arduino: millis() is the native source
  #include <Arduino.h>
  inline TIME plc_millis() { return (TIME)millis(); }
  // T() overload: T(5000) style
  inline constexpr TIME T(uint32_t ms) { return ms; }
#else
  // Linux / Raspberry Pi: steady_clock (monotonic, no leap-second jumps)
  #include <chrono>
  #include <string>
  using STRING = std::string;

  inline TIME plc_millis() {
      return (TIME)std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch()).count();
  }

  // T(5s), T(500ms) — keeps ST literal style on the PC/RPi side
  using namespace std::chrono_literals;
  inline TIME T(std::chrono::milliseconds t) { return (TIME)t.count(); }
  inline constexpr TIME T(uint32_t ms)       { return ms; }
#endif
