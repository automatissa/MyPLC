#pragma once
#include "plc/types.h"

namespace myplc {

// ============================================================================
//  R_TRIG — Rising Edge Detector
// ============================================================================
//  Structured Text equivalent:
//
//    myEdge : R_TRIG;
//    myEdge(CLK := start_button);
//    IF myEdge.Q THEN  (* executes exactly once per button press *)
//      counter := counter + 1;
//    END_IF
//
//  Behaviour:
//    - Q is TRUE for exactly ONE scan cycle when CLK transitions FALSE → TRUE
//    - Q is FALSE on all other cycles
// ============================================================================
class R_TRIG {
    BOOL _CLK = false;
    BOOL _Q   = false;
public:
    void operator()(BOOL clk) { CLK(clk); }
    void CLK(BOOL clk);
    BOOL Q() const { return _Q; }
};

}  // namespace myplc
