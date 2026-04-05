#include "plc/triggers/F_TRIG.h"

namespace myplc {

void F_TRIG::CLK(BOOL clk) {
    _Q   = !clk && _CLK;   // TRUE only on the transition TRUE → FALSE
    _CLK = clk;             // remember current state for next scan
}

}  // namespace myplc
