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
        _start   = plc_millis();
        _ET      = 0;
        _Q       = false;
        _running = true;
    }
    // Falling edge: reset immediately
    if (!_IN && _prev_IN) {
        _ET      = 0;
        _Q       = false;
        _running = false;
    }
    _prev_IN = _IN;

    // Update elapsed time while running
    if (_running) {
        _ET = plc_millis() - _start;
        if (_ET >= _PT) {
            _ET      = _PT;   // clamp to preset
            _Q       = true;
            _running = false;
        }
    }
}

}  // namespace myplc
