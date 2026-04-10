#include "plc/timers/TP.h"

namespace myplc {

void TP::_execute() {
    // Rising edge AND timer not currently running: start the pulse
    if (_IN && !_prev_IN && !_running) {
        _start   = plc_millis();
        _ET      = 0;
        _Q       = true;
        _running = true;
    }
    // NOTE: falling edge (_IN going FALSE) has NO effect on TP output.
    //       The timer runs to completion regardless of IN state.
    _prev_IN = _IN;

    if (_running) {
        _ET = plc_millis() - _start;
        if (_ET >= _PT) {
            _ET      = _PT;
            _Q       = false;
            _running = false;
        }
    }
}

}  // namespace myplc
