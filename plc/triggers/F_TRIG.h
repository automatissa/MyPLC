#pragma once
#include "plc/types.h"

namespace myplc {

// ============================================================================
//  F_TRIG — Falling Edge Detector
// ============================================================================
//  Structured Text equivalent:
//
//    myEdge : F_TRIG;
//    myEdge(CLK := run_signal);
//    IF myEdge.Q THEN  (* executes exactly once when signal goes OFF *)
//      alarm := TRUE;
//    END_IF
//
//  Behaviour:
//    - Q is TRUE for exactly ONE scan cycle when CLK transitions TRUE → FALSE
//    - Q is FALSE on all other cycles
// ============================================================================
class F_TRIG {
    BOOL _CLK = false;
    BOOL _Q   = false;
public:
    void operator()(BOOL clk) { CLK(clk); }
    void CLK(BOOL clk);
    BOOL Q() const { return _Q; }
};

}  // namespace myplc
