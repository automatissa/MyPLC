#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

// ============================================================================
//  Variable Registry — powers the web simulator dashboard.
//
//  Each PLC_VAR declaration automatically registers the variable so the
//  dashboard can display and write it in real time.
//
//  Usage in user/program.cpp:
//    PLC_VAR(BOOL, start_button, false)
//    PLC_VAR(INT,  setpoint,     100)
//    PLC_VAR(REAL, temperature,  0.0f)
// ============================================================================

namespace myplc::sim {

struct VarInfo {
    std::string name;
    std::string type;
    std::function<std::string()>            get;   // read current value as string
    std::function<void(const std::string&)> set;   // write value from string
};

class Registry {
    std::vector<VarInfo> _vars;
    Registry() = default;
public:
    static Registry& get();
    void        add(VarInfo v);
    std::string to_json() const;
    bool        write(const std::string& name, const std::string& value);
};

// ── Type ↔ string converters (one per supported PLC type) ─────────────────
inline std::string to_str(bool v)               { return v ? "true" : "false"; }
inline std::string to_str(int16_t v)            { return std::to_string(v); }
inline std::string to_str(int32_t v)            { return std::to_string(v); }
inline std::string to_str(int64_t v)            { return std::to_string(v); }
inline std::string to_str(uint8_t v)            { return std::to_string(v); }
inline std::string to_str(uint16_t v)           { return std::to_string(v); }
inline std::string to_str(uint32_t v)           { return std::to_string(v); }
inline std::string to_str(float v)              { std::ostringstream ss; ss << v; return ss.str(); }
inline std::string to_str(double v)             { std::ostringstream ss; ss << v; return ss.str(); }
inline std::string to_str(const std::string& v) { return v; }

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

}  // namespace myplc::sim

// ============================================================================
//  PLC_VAR(TYPE, name, initial)
//
//  Declares a global PLC variable of the given type and registers it with
//  the simulator dashboard.  Must be used at file scope in a .cpp file.
//
//  The variable is accessible by name inside INIT() and LOOP() in the same
//  translation unit (user/program.cpp).
//
//  Examples:
//    PLC_VAR(BOOL, start_button, false)
//    PLC_VAR(INT,  production_count, 0)
//    PLC_VAR(REAL, tank_level, 0.0f)
// ============================================================================
#define PLC_VAR(TYPE, name, initial)                                          \
    TYPE name = (initial);                                                    \
    namespace {                                                               \
        struct _PlcReg_##name {                                               \
            _PlcReg_##name() {                                                \
                myplc::sim::Registry::get().add({                            \
                    #name, #TYPE,                                             \
                    []() -> std::string {                                     \
                        return myplc::sim::to_str(name);                     \
                    },                                                        \
                    [](const std::string& _v) {                              \
                        name = myplc::sim::from_str<TYPE>(_v);               \
                    }                                                         \
                });                                                           \
            }                                                                 \
        } _plc_reg_##name;                                                    \
    }
