#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <cstring>
#include <chrono>   // for std::chrono::milliseconds (= TIME on PC/RPi)

// ============================================================================
//  Variable Registry — powers the web dashboard + Modbus server.
//
//  Each VAR_INPUT / VAR_OUTPUT / VAR_MEM declaration automatically:
//    1. Registers the variable with the web dashboard
//    2. Assigns a Modbus holding-register address (auto-incremented)
//
//  Register layout (big-endian, Modbus standard):
//    BOOL, BYTE, WORD, INT, UINT    → 1 register
//    DWORD, DINT, UDINT, REAL, TIME → 2 registers  (high word first)
//    LINT, LREAL                    → 4 registers  (high word first)
// ============================================================================

namespace myplc::sim {

// ── Modbus register count per type ───────────────────────────────────────────
template<typename T> inline uint8_t mb_reg_count()          { return 2; }  // 32-bit default
template<> inline uint8_t mb_reg_count<bool>()              { return 1; }
template<> inline uint8_t mb_reg_count<uint8_t>()           { return 1; }
template<> inline uint8_t mb_reg_count<int16_t>()           { return 1; }
template<> inline uint8_t mb_reg_count<uint16_t>()          { return 1; }
template<> inline uint8_t mb_reg_count<int32_t>()           { return 2; }
template<> inline uint8_t mb_reg_count<uint32_t>()          { return 2; }
template<> inline uint8_t mb_reg_count<float>()             { return 2; }
template<> inline uint8_t mb_reg_count<int64_t>()           { return 4; }
template<> inline uint8_t mb_reg_count<double>()            { return 4; }
template<> inline uint8_t mb_reg_count<std::chrono::milliseconds>() { return 2; }

// ── Modbus big-endian encode: T → uint16_t registers ─────────────────────────
template<typename T> inline void mb_encode(const T&, uint16_t*) {}  // unimplemented fallback

template<> inline void mb_encode<bool>    (const bool& v,     uint16_t* r) { r[0] = v ? 1u : 0u; }
template<> inline void mb_encode<uint8_t> (const uint8_t& v,  uint16_t* r) { r[0] = v; }
template<> inline void mb_encode<int16_t> (const int16_t& v,  uint16_t* r) { r[0] = static_cast<uint16_t>(v); }
template<> inline void mb_encode<uint16_t>(const uint16_t& v, uint16_t* r) { r[0] = v; }

template<> inline void mb_encode<int32_t>(const int32_t& v, uint16_t* r) {
    uint32_t u; std::memcpy(&u, &v, 4);
    r[0] = static_cast<uint16_t>(u >> 16);
    r[1] = static_cast<uint16_t>(u & 0xFFFFu);
}
template<> inline void mb_encode<uint32_t>(const uint32_t& v, uint16_t* r) {
    r[0] = static_cast<uint16_t>(v >> 16);
    r[1] = static_cast<uint16_t>(v & 0xFFFFu);
}
template<> inline void mb_encode<float>(const float& v, uint16_t* r) {
    uint32_t u; std::memcpy(&u, &v, 4);
    r[0] = static_cast<uint16_t>(u >> 16);
    r[1] = static_cast<uint16_t>(u & 0xFFFFu);
}
template<> inline void mb_encode<std::chrono::milliseconds>(const std::chrono::milliseconds& v, uint16_t* r) {
    uint32_t ms = static_cast<uint32_t>(v.count());
    r[0] = static_cast<uint16_t>(ms >> 16);
    r[1] = static_cast<uint16_t>(ms & 0xFFFFu);
}

template<> inline void mb_encode<int64_t>(const int64_t& v, uint16_t* r) {
    uint64_t u; std::memcpy(&u, &v, 8);
    r[0] = static_cast<uint16_t>(u >> 48);
    r[1] = static_cast<uint16_t>((u >> 32) & 0xFFFFu);
    r[2] = static_cast<uint16_t>((u >> 16) & 0xFFFFu);
    r[3] = static_cast<uint16_t>(u & 0xFFFFu);
}
template<> inline void mb_encode<double>(const double& v, uint16_t* r) {
    uint64_t u; std::memcpy(&u, &v, 8);
    r[0] = static_cast<uint16_t>(u >> 48);
    r[1] = static_cast<uint16_t>((u >> 32) & 0xFFFFu);
    r[2] = static_cast<uint16_t>((u >> 16) & 0xFFFFu);
    r[3] = static_cast<uint16_t>(u & 0xFFFFu);
}

// ── Modbus big-endian decode: uint16_t registers → T ─────────────────────────
template<typename T> inline T mb_decode(const uint16_t*) { return T{}; }

template<> inline bool     mb_decode<bool>    (const uint16_t* r) { return r[0] != 0u; }
template<> inline uint8_t  mb_decode<uint8_t> (const uint16_t* r) { return static_cast<uint8_t>(r[0]); }
template<> inline int16_t  mb_decode<int16_t> (const uint16_t* r) { return static_cast<int16_t>(r[0]); }
template<> inline uint16_t mb_decode<uint16_t>(const uint16_t* r) { return r[0]; }

template<> inline int32_t mb_decode<int32_t>(const uint16_t* r) {
    uint32_t u = (static_cast<uint32_t>(r[0]) << 16) | r[1];
    int32_t v; std::memcpy(&v, &u, 4); return v;
}
template<> inline uint32_t mb_decode<uint32_t>(const uint16_t* r) {
    return (static_cast<uint32_t>(r[0]) << 16) | r[1];
}
template<> inline float mb_decode<float>(const uint16_t* r) {
    uint32_t u = (static_cast<uint32_t>(r[0]) << 16) | r[1];
    float v; std::memcpy(&v, &u, 4); return v;
}
template<> inline std::chrono::milliseconds mb_decode<std::chrono::milliseconds>(const uint16_t* r) {
    uint32_t ms = (static_cast<uint32_t>(r[0]) << 16) | r[1];
    return std::chrono::milliseconds(ms);
}
template<> inline int64_t mb_decode<int64_t>(const uint16_t* r) {
    uint64_t u = (static_cast<uint64_t>(r[0]) << 48)
               | (static_cast<uint64_t>(r[1]) << 32)
               | (static_cast<uint64_t>(r[2]) << 16)
               |  static_cast<uint64_t>(r[3]);
    int64_t v; std::memcpy(&v, &u, 8); return v;
}
template<> inline double mb_decode<double>(const uint16_t* r) {
    uint64_t u = (static_cast<uint64_t>(r[0]) << 48)
               | (static_cast<uint64_t>(r[1]) << 32)
               | (static_cast<uint64_t>(r[2]) << 16)
               |  static_cast<uint64_t>(r[3]);
    double v; std::memcpy(&v, &u, 8); return v;
}

// ── Type ↔ string converters ──────────────────────────────────────────────────
inline std::string to_str(bool v)                        { return v ? "true" : "false"; }
inline std::string to_str(int16_t v)                     { return std::to_string(v); }
inline std::string to_str(int32_t v)                     { return std::to_string(v); }
inline std::string to_str(int64_t v)                     { return std::to_string(v); }
inline std::string to_str(uint8_t v)                     { return std::to_string(v); }
inline std::string to_str(uint16_t v)                    { return std::to_string(v); }
inline std::string to_str(uint32_t v)                    { return std::to_string(v); }
inline std::string to_str(float v)                       { std::ostringstream ss; ss << v; return ss.str(); }
inline std::string to_str(double v)                      { std::ostringstream ss; ss << v; return ss.str(); }
inline std::string to_str(const std::string& v)          { return v; }
inline std::string to_str(std::chrono::milliseconds v)   { return std::to_string(v.count()); }

template<typename T> T from_str(const std::string& s);
template<> inline bool        from_str<bool>       (const std::string& s) { return s == "true" || s == "1"; }
template<> inline int16_t     from_str<int16_t>    (const std::string& s) { return static_cast<int16_t>(std::stoi(s)); }
template<> inline int32_t     from_str<int32_t>    (const std::string& s) { return static_cast<int32_t>(std::stol(s)); }
template<> inline int64_t     from_str<int64_t>    (const std::string& s) { return static_cast<int64_t>(std::stoll(s)); }
template<> inline uint8_t     from_str<uint8_t>    (const std::string& s) { return static_cast<uint8_t>(std::stoul(s)); }
template<> inline uint16_t    from_str<uint16_t>   (const std::string& s) { return static_cast<uint16_t>(std::stoul(s)); }
template<> inline uint32_t    from_str<uint32_t>   (const std::string& s) { return static_cast<uint32_t>(std::stoul(s)); }
template<> inline float       from_str<float>      (const std::string& s) { return std::stof(s); }
template<> inline double      from_str<double>     (const std::string& s) { return std::stod(s); }
template<> inline std::string from_str<std::string>(const std::string& s) { return s; }
template<> inline std::chrono::milliseconds from_str<std::chrono::milliseconds>(const std::string& s) {
    return std::chrono::milliseconds(std::stoll(s));
}

// ── Variable direction ────────────────────────────────────────────────────────
enum class VarDir { INPUT, OUTPUT, MEM };

inline const char* dir_str(VarDir d) {
    switch (d) {
        case VarDir::INPUT:  return "INPUT";
        case VarDir::OUTPUT: return "OUTPUT";
        default:             return "MEM";
    }
}

// ── VarInfo ───────────────────────────────────────────────────────────────────
struct VarInfo {
    std::string name;
    std::string type;
    VarDir      dir      = VarDir::MEM;  // INPUT / OUTPUT / MEM
    uint16_t    mb_addr  = 0;            // first Modbus holding register address
    uint8_t     mb_count = 1;            // number of registers this variable occupies

    std::function<std::string()>             get;       // read as string (dashboard)
    std::function<void(const std::string&)>  set;       // write from string (dashboard)
    std::function<void(uint16_t*)>           get_regs;  // encode → Modbus registers
    std::function<void(const uint16_t*)>     set_regs;  // decode ← Modbus registers
};

// ── Registry ──────────────────────────────────────────────────────────────────
class Registry {
    std::vector<VarInfo> _vars;
    uint16_t             _next_addr = 0;
    bool                 _sim_mode  = true;  // true=SIMU (dashboard writes INPUT), false=RÉEL (read-only)
    Registry() = default;
public:
    static Registry& get();

    void        add(VarInfo v);
    std::string to_json() const;
    bool        write(const std::string& name, const std::string& value);

    // Simulation / real mode toggle (dashboard button)
    bool toggle_sim_mode() { _sim_mode = !_sim_mode; return _sim_mode; }
    bool sim_mode()  const { return _sim_mode; }

    // Modbus bulk register access
    void fill_response_regs(uint16_t start, uint16_t count, uint16_t* buf) const;
    void apply_write_regs  (uint16_t start, uint16_t count, const uint16_t* data);

    // Print variable → Modbus address table to stdout
    void print_mb_map() const;

    uint16_t total_regs() const { return _next_addr; }
};

}  // namespace myplc::sim

// ============================================================================
//  Variable declaration macros:
//
//    VAR_INPUT (TYPE, name, initial)  — physical input  (ESP32 → RPi)   writable
//    VAR_OUTPUT(TYPE, name, initial)  — physical output (RPi  → ESP32)  read-only
//    VAR_MEM   (TYPE, name, initial)  — internal memory                  writable
//
//  Each macro:
//    1. Declares the global variable
//    2. Registers it with the web dashboard
//    3. Assigns it a Modbus holding-register address (auto-incremented)
//
//  VAR_INPUT  → writable from dashboard and Modbus (ESP32 pushes sensor values here)
//  VAR_OUTPUT → read-only (computed by the PLC — ESP32 reads actuator commands here)
//  VAR_MEM    → internal state — readable and writable from dashboard and Modbus
// ============================================================================

// Internal helper — do not use directly.
#define _PLCVAR_IMPL(TYPE, _nm_, initial, _dir_)                                  \
    TYPE _nm_ = (initial);                                                        \
    namespace {                                                                   \
        struct _PlcReg_##_nm_ {                                                   \
            _PlcReg_##_nm_() {                                                    \
                myplc::sim::VarInfo _vi;                                          \
                _vi.name     = #_nm_;                                             \
                _vi.type     = #TYPE;                                             \
                _vi.dir      = (_dir_);                                           \
                _vi.mb_count = myplc::sim::mb_reg_count<TYPE>();                 \
                _vi.get      = []() -> std::string {                              \
                    return myplc::sim::to_str(_nm_);                              \
                };                                                                \
                _vi.set      = [](const std::string& _v) {                        \
                    _nm_ = myplc::sim::from_str<TYPE>(_v);                        \
                };                                                                \
                _vi.get_regs = [](uint16_t* _r) {                                \
                    myplc::sim::mb_encode<TYPE>(_nm_, _r);                        \
                };                                                                \
                _vi.set_regs = [](const uint16_t* _r) {                          \
                    _nm_ = myplc::sim::mb_decode<TYPE>(_r);                       \
                };                                                                \
                myplc::sim::Registry::get().add(std::move(_vi));                  \
            }                                                                     \
        } _plc_reg_##_nm_;                                                        \
    }

#define VAR_INPUT(TYPE,  _nm_, initial) _PLCVAR_IMPL(TYPE, _nm_, initial, myplc::sim::VarDir::INPUT)
#define VAR_OUTPUT(TYPE, _nm_, initial) _PLCVAR_IMPL(TYPE, _nm_, initial, myplc::sim::VarDir::OUTPUT)
#define VAR_MEM(TYPE,    _nm_, initial) _PLCVAR_IMPL(TYPE, _nm_, initial, myplc::sim::VarDir::MEM)
