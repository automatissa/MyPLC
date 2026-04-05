#include "plc/counters/CTD.h"

namespace myplc {

void CTD::operator()(BOOL cd, BOOL load, INT pv) {
    _PV = pv;
    LD(load);    // load takes priority
    CD(cd);
}

void CTD::CD(BOOL cd) {
    if (cd && !_prev_CD) {    // decrement on rising edge only
        _CV--;
    }
    _prev_CD = cd;
    _Q = (_CV <= 0);
}

void CTD::LD(BOOL load) {
    if (load) {
        _CV      = _PV;
        _prev_CD = false;
        _Q       = false;
    }
}

}  // namespace myplc
