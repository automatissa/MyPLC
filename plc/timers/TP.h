#pragma once
#include "plc/timers/TON.h"

namespace myplc {

// ============================================================================
//  TP — Timer Pulse  (non-retriggerable one-shot)
// ============================================================================
//  Structured Text equivalent:
//
//    myPulse : TP;
//    myPulse(IN := trigger, PT := T#2s);
//    signal := myPulse.Q;   (* TRUE for exactly 2 s on each rising edge *)
//
//  Behaviour:
//    - Rising edge of IN while NOT running → Q=TRUE, starts PT timer
//    - IN going FALSE while running        → NO effect (non-retriggerable)
//    - Second rising edge while running    → IGNORED (non-retriggerable)
//    - When ET >= PT                       → Q=FALSE; new trigger now possible
//
//  Key difference from TON: once triggered, IN state is irrelevant until done.
// ============================================================================
class TP : public TON {
protected:
    void _execute() override;
};

}  // namespace myplc
