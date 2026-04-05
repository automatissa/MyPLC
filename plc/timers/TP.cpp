#include "plc/timers/TP.h"

namespace myplc {

void TP::_execute() {
    // Rising edge AND timer not currently running: start the pulse
    if (_IN && !_prev_IN && !_running) {
        _start   = std::chrono::steady_clock::now();
        _ET      = 0ms;
        _Q       = true;
        _running = true;
    }
    // NOTE: falling edge (_IN going FALSE) has NO effect on TP output.
    //       The timer runs to completion regardless of IN state.
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
