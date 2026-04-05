#pragma once
#include "plc/types.h"

namespace myplc {

// ============================================================================
//  CTD — Count Down Counter
// ============================================================================
//  Structured Text equivalent:
//
//    myCounter : CTD;
//    myCounter(CD := pulse, LD := load_btn, PV := 10);
//    IF myCounter.Q THEN   (* reached zero *)
//      stop_machine := TRUE;
//    END_IF
//
//  Behaviour:
//    - LD=TRUE loads CV := PV (initialise to preset)
//    - CV decrements on every rising edge of CD
//    - Q becomes TRUE when CV <= 0
// ============================================================================
class CTD {
    BOOL _CD      = false;
    BOOL _prev_CD = false;
    BOOL _Q       = false;
    INT  _PV      = 0;
    INT  _CV      = 0;
public:
    void operator()(BOOL cd, BOOL load, INT pv);

    void CD(BOOL cd);
    void LD(BOOL load);
    void PV(INT pv)    { _PV = pv; }

    BOOL Q()  const    { return _Q; }
    INT  CV() const    { return _CV; }
};

}  // namespace myplc
