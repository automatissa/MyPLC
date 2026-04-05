#include "plc/timers/TON.h"

namespace myplc {

void TON::operator()(BOOL in, TIME pt) {
    _PT = pt;
    IN(in);
}

void TON::operator()(BOOL in) {
    IN(in);
}

void TON::IN(BOOL in) {
    _IN = in;
    _execute();
}

void TON::_execute() {
    // Rising edge: start timing
    if (_IN && !_prev_IN) {
        _start   = std::chrono::steady_clock::now();
        _ET      = 0ms;
        _Q       = false;
        _running = true;
    }
    // Falling edge: reset immediately
    if (!_IN && _prev_IN) {
        _ET      = 0ms;
        _Q       = false;
        _running = false;
    }
    _prev_IN = _IN;

    // Update elapsed time while running
    if (_running) {
        auto now = std::chrono::steady_clock::now();
        _ET = std::chrono::duration_cast<TIME>(now - _start);
        if (_ET >= _PT) {
            _ET      = _PT;   // clamp to preset
            _Q       = true;
            _running = false;
        }
    }
}

}  // namespace myplc
