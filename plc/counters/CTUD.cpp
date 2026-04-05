#include "plc/counters/CTUD.h"

namespace myplc {

void CTUD::operator()(BOOL cu, BOOL cd, BOOL reset, BOOL load, INT pv) {
    _PV = pv;
    R(reset);
    LD(load);
    CU(cu);
    CD(cd);
}

void CTUD::CU(BOOL cu) {
    if (cu && !_prev_CU) _CV++;
    _prev_CU = cu;
}

void CTUD::CD(BOOL cd) {
    if (cd && !_prev_CD) _CV--;
    _prev_CD = cd;
}

void CTUD::R(BOOL reset) {
    if (reset) {
        _CV      = 0;
        _prev_CU = false;
        _prev_CD = false;
    }
}

void CTUD::LD(BOOL load) {
    if (load) {
        _CV      = _PV;
        _prev_CU = false;
        _prev_CD = false;
    }
}

}  // namespace myplc
