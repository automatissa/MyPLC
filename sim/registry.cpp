#include "sim/registry.h"
#include <cstring>
#include <cstdio>

namespace myplc::sim {

Registry& Registry::get() {
    static Registry instance;
    return instance;
}

void Registry::add(VarInfo v) {
    v.mb_addr  = _next_addr;
    _next_addr = static_cast<uint16_t>(_next_addr + v.mb_count);
    _vars.push_back(std::move(v));
}

std::string Registry::to_json() const {
    std::ostringstream ss;
    ss << "{\"sim\":" << (_sim_mode ? "true" : "false") << ",\"vars\":[";
    for (size_t i = 0; i < _vars.size(); ++i) {
        if (i > 0) ss << ",";
        const auto& v = _vars[i];
        std::string val = v.get();
        std::string escaped;
        for (char c : val) {
            if      (c == '"')  escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else                escaped += c;
        }
        ss << "{\"name\":\"" << v.name << "\","
           << "\"type\":\"" << v.type << "\","
           << "\"dir\":\"" << dir_str(v.dir) << "\","
           << "\"mb_addr\":" << v.mb_addr << ","
           << "\"mb_count\":" << static_cast<int>(v.mb_count) << ","
           << "\"value\":\"" << escaped << "\"}";
    }
    ss << "]}";
    return ss.str();
}

bool Registry::write(const std::string& name, const std::string& value) {
    for (auto& v : _vars) {
        if (v.name == name) {
            if (v.dir == VarDir::OUTPUT) return false;              // OUTPUT is read-only
            if (v.dir == VarDir::INPUT && !_sim_mode) return false; // INPUT locked in RÉEL mode
            try   { v.set(value); return true; }
            catch (...) { return false; }
        }
    }
    return false;
}

void Registry::fill_response_regs(uint16_t start, uint16_t count, uint16_t* buf) const {
    std::memset(buf, 0, count * sizeof(uint16_t));
    for (const auto& v : _vars) {
        uint16_t vend = static_cast<uint16_t>(v.mb_addr + v.mb_count);
        uint16_t rend = static_cast<uint16_t>(start + count);
        if (v.mb_addr >= rend || vend <= start) continue;
        uint16_t tmp[4] = {};
        v.get_regs(tmp);
        for (uint8_t i = 0; i < v.mb_count; ++i) {
            uint16_t abs = static_cast<uint16_t>(v.mb_addr + i);
            if (abs >= start && abs < rend)
                buf[abs - start] = tmp[i];
        }
    }
}

void Registry::apply_write_regs(uint16_t start, uint16_t count, const uint16_t* data) {
    uint16_t rend = static_cast<uint16_t>(start + count);
    for (auto& v : _vars) {
        if (v.dir == VarDir::OUTPUT) continue;  // OUTPUT is read-only
        uint16_t vend = static_cast<uint16_t>(v.mb_addr + v.mb_count);
        if (v.mb_addr >= rend || vend <= start) continue;
        if (v.mb_addr >= start && vend <= rend) {
            // Fully contained — apply directly
            v.set_regs(data + (v.mb_addr - start));
        } else {
            // Partial overlap — read-modify-write
            uint16_t tmp[4] = {};
            v.get_regs(tmp);
            for (uint8_t i = 0; i < v.mb_count; ++i) {
                uint16_t abs = static_cast<uint16_t>(v.mb_addr + i);
                if (abs >= start && abs < rend)
                    tmp[i] = data[abs - start];
            }
            v.set_regs(tmp);
        }
    }
}

void Registry::print_mb_map() const {
    // Format standard industriel : adresse décimale base-0 (= adresse protocole)
    // et notation Modbus 4xxxx (holding registers, affichage seulement)
    std::printf("  Holding Registers — FC03 read / FC06 FC16 write\n");
    std::printf("  %-24s %-8s  %5s  %5s  %s\n",
                "Variable", "Type", "Addr", "4xxxx", "Regs");
    std::printf("  %-24s %-8s  %5s  %5s  %s\n",
                "------------------------", "--------",
                "-----", "-------", "----");
    for (const auto& v : _vars) {
        std::printf("  %-24s %-8s  %5d  4%04d  %d\n",
                    v.name.c_str(), v.type.c_str(),
                    static_cast<int>(v.mb_addr),
                    static_cast<int>(v.mb_addr + 1),  // Modbus 4xxxx notation (1-based display)
                    static_cast<int>(v.mb_count));
    }
    std::printf("\n");
}

}  // namespace myplc::sim
