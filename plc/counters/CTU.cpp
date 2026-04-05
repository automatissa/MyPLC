#include "plc/counters/CTU.h"

namespace myplc {

void CTU::operator()(BOOL cu, BOOL reset, INT pv) {
    _PV = pv;
    R(reset);    // reset takes priority (evaluated first)
    CU(cu);
}

void CTU::CU(BOOL cu) {
    if (cu && !_prev_CU) {    // count on rising edge only
        _CV++;
    }
    _prev_CU = cu;
    _Q = (_CV >= _PV);
}

void CTU::R(BOOL reset) {
    if (reset) {
        _CV      = 0;
        _prev_CU = false;
        _Q       = false;
        // NOTE: _PV is intentionally NOT reset, so the target stays valid.
    }
}

}  // namespace myplc
