#include "plc/triggers/R_TRIG.h"

namespace myplc {

void R_TRIG::CLK(BOOL clk) {
    _Q   = clk && !_CLK;   // TRUE only on the transition FALSE → TRUE
    _CLK = clk;             // remember current state for next scan
}

}  // namespace myplc
