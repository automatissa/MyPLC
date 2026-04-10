#include "plc/timers/TOF.h"

namespace myplc {

void TOF::_execute() {
    // Rising edge: Q=TRUE, cancel any running off-delay
    if (_IN && !_prev_IN) {
        _ET      = 0;
        _Q       = true;
        _running = false;
    }
    // Falling edge: keep Q=TRUE, start off-delay timer
    if (!_IN && _prev_IN) {
        _start   = plc_millis();
        _ET      = 0;
        _Q       = true;
        _running = true;
    }
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
