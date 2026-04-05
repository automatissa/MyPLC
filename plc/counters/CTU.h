#pragma once
#include "plc/types.h"

namespace myplc {

// ============================================================================
//  CTU — Count Up Counter
// ============================================================================
//  Structured Text equivalent:
//
//    myCounter : CTU;
//    myCounter(CU := pulse, R := reset_btn, PV := 10);
//    IF myCounter.Q THEN   (* target reached *)
//      alarm := TRUE;
//    END_IF
//    display := myCounter.CV;
//
//  Behaviour:
//    - CV increments on every rising edge of CU
//    - Q becomes TRUE when CV >= PV
//    - R=TRUE resets CV to 0 (PV is kept intact)
// ============================================================================
class CTU {
    BOOL _CU      = false;
    BOOL _prev_CU = false;
    BOOL _Q       = false;
    INT  _PV      = 0;
    INT  _CV      = 0;
public:
    // Callable operator — mirrors ST call syntax
    void operator()(BOOL cu, BOOL reset, INT pv);

    void CU(BOOL cu);
    void R(BOOL reset);
    void PV(INT pv)    { _PV = pv; }

    BOOL Q()  const    { return _Q; }
    INT  CV() const    { return _CV; }
};

}  // namespace myplc
