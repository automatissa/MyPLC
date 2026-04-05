#pragma once
#include "plc/timers/TON.h"

namespace myplc {

// ============================================================================
//  TOF — Timer Off Delay
// ============================================================================
//  Structured Text equivalent:
//
//    myTimer : TOF;
//    myTimer(IN := sensor, PT := T#3s);
//    lamp := myTimer.Q;   (* TRUE while sensor TRUE + 3 s after sensor goes FALSE *)
//
//  Behaviour:
//    - Rising edge of IN  → Q=TRUE immediately, stops any running timer
//    - Falling edge of IN → Q stays TRUE, starts off-delay timer
//    - When ET >= PT      → Q becomes FALSE
// ============================================================================
class TOF : public TON {
protected:
    void _execute() override;
};

}  // namespace myplc
