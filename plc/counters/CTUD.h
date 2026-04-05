#pragma once
#include "plc/types.h"

namespace myplc {

// ============================================================================
//  CTUD — Count Up / Down Counter
// ============================================================================
//  Structured Text equivalent:
//
//    myCounter : CTUD;
//    myCounter(CU := up_btn, CD := down_btn, R := reset_btn,
//              LD := load_btn, PV := 10);
//    IF myCounter.QU THEN  (* reached upper limit *)
//      full_alarm := TRUE;
//    END_IF
//    IF myCounter.QD THEN  (* reached zero *)
//      empty_alarm := TRUE;
//    END_IF
//
//  Behaviour:
//    - CU rising edge → CV increments
//    - CD rising edge → CV decrements
//    - R=TRUE → CV := 0
//    - LD=TRUE → CV := PV
//    - QU is TRUE when CV >= PV
//    - QD is TRUE when CV <= 0
// ============================================================================
class CTUD {
    BOOL _prev_CU = false;
    BOOL _prev_CD = false;
    INT  _PV      = 0;
    INT  _CV      = 0;
public:
    void operator()(BOOL cu, BOOL cd, BOOL reset, BOOL load, INT pv);

    void CU(BOOL cu);
    void CD(BOOL cd);
    void R(BOOL reset);
    void LD(BOOL load);
    void PV(INT pv) { _PV = pv; }

    BOOL QU() const { return _CV >= _PV; }
    BOOL QD() const { return _CV <= 0;   }
    INT  CV() const { return _CV;        }
};

}  // namespace myplc
