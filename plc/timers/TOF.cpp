#include "plc/timers/TOF.h"

namespace myplc {

void TOF::_execute() {
    // Rising edge: Q=TRUE, cancel any running off-delay
    if (_IN && !_prev_IN) {
        _ET      = 0ms;
        _Q       = true;
        _running = false;
    }
    // Falling edge: keep Q=TRUE, start off-delay timer
    if (!_IN && _prev_IN) {
        _start   = std::chrono::steady_clock::now();
        _ET      = 0ms;
        _Q       = true;
        _running = true;
    }
    _prev_IN = _IN;

    if (_running) {
        auto now = std::chrono::steady_clock::now();
        _ET = std::chrono::duration_cast<TIME>(now - _start);
        if (_ET >= _PT) {
            _ET      = _PT;
            _Q       = false;
            _running = false;
        }
    }
}

}  // namespace myplc
