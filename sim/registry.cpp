#include "sim/registry.h"

namespace myplc::sim {

Registry& Registry::get() {
    static Registry instance;
    return instance;
}

void Registry::add(VarInfo v) {
    _vars.push_back(std::move(v));
}

std::string Registry::to_json() const {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < _vars.size(); ++i) {
        if (i > 0) ss << ",";
        const auto& v = _vars[i];
        // Escape the value string so JSON stays valid
        std::string val = v.get();
        std::string escaped;
        for (char c : val) {
            if      (c == '"')  escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else                escaped += c;
        }
        ss << "{\"name\":\"" << v.name << "\","
           << "\"type\":\"" << v.type << "\","
           << "\"value\":\"" << escaped << "\"}";
    }
    ss << "]";
    return ss.str();
}

bool Registry::write(const std::string& name, const std::string& value) {
    for (auto& v : _vars) {
        if (v.name == name) {
            try   { v.set(value); return true;  }
            catch (...) { return false; }
        }
    }
    return false;
}

}  // namespace myplc::sim
