#include "sim/registry.h"
#include <cstring>

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
    ss << "[";
    for (size_t i = 0; i < _vars.size(); ++i) {
        if (i > 0) ss << ",";
        const auto& v = _vars[i];
        std::string val = v.get();
        // Escape value for valid JSON
        std::string escaped;
        for (char c : val) {
            if      (c == '"')  escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else                escaped += c;
        }
        ss << "{\"name\":\"" << v.name << "\","
           << "\"type\":\"" << v.type << "\","
           << "\"mb_addr\":" << v.mb_addr << ","
           << "\"mb_count\":" << static_cast<int>(v.mb_count) << ","
           << "\"value\":\"" << escaped << "\"}";
    }
    ss << "]";
    return ss.str();
}

bool Registry::write(const std::string& name, const std::string& value) {
    for (auto& v : _vars) {
        if (v.name == name) {
            try {
                v.set(value);
                v.dirty = true;
                return true;
            } catch (...) { return false; }
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
            uint16_t abs_addr = static_cast<uint16_t>(v.mb_addr + i);
            if (abs_addr >= start && abs_addr < rend) {
                buf[abs_addr - start] = tmp[i];
            }
        }
    }
}

void Registry::apply_write_regs(uint16_t start, uint16_t count, const uint16_t* data) {
    uint16_t rend = static_cast<uint16_t>(start + count);

    for (auto& v : _vars) {
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
                uint16_t abs_addr = static_cast<uint16_t>(v.mb_addr + i);
                if (abs_addr >= start && abs_addr < rend) {
                    tmp[i] = data[abs_addr - start];
                }
            }
            v.set_regs(tmp);
        }
    }
}

void Registry::drain_dirty(std::function<void(uint16_t, uint8_t, const uint16_t*)> cb) {
    for (auto& v : _vars) {
        if (!v.dirty) continue;
        uint16_t tmp[4] = {};
        v.get_regs(tmp);
        cb(v.mb_addr, v.mb_count, tmp);
        v.dirty = false;
    }
}

}  // namespace myplc::sim
