#pragma once
#include <chrono>
#include <cstdint>
#include <string>

// ============================================================================
//  IEC 61131-3 Standard Data Types
//  Map C++ primitives to the names used in Structured Text (ST).
//  Using these types makes C++ code read closer to ST programs.
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
using STRING = std::string;
using TIME   = std::chrono::milliseconds;

// Enable chrono literals: 5s, 500ms, 1min, ...
using namespace std::chrono_literals;

// TIME constructor helper — mirrors the T#... literal syntax in ST.
// Usage:  T(5s)   →  5000 ms
//         T(500ms) →   500 ms
inline TIME T(TIME t) { return t; }
