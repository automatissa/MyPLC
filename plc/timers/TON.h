#pragma once
#include "plc/types.h"
#include <chrono>

namespace myplc {

// ============================================================================
//  TON — Timer On Delay
// ============================================================================
//  Structured Text equivalent:
//
//    myTimer : TON;
//    myTimer(IN := sensor, PT := T#5s);
//    output := myTimer.Q;    (* TRUE after sensor held TRUE for 5 s *)
//
//  Behaviour:
//    - Rising edge of IN  → starts timing, ET counts up
//    - Falling edge of IN → resets ET and Q immediately
//    - When ET >= PT      → Q becomes TRUE, timing stops
// ============================================================================
class TON {
public:
    // Callable operator — mirrors ST call syntax:  myTimer(sensor, T(5s));
    void operator()(BOOL in, TIME pt);
    void operator()(BOOL in);       // PT was already set with PT()

    // Individual input setters (for multi-line ST style)
    void IN(BOOL in);
    void PT(TIME pt) { _PT = pt; }

    // Output getters
    BOOL Q()  const { return _Q; }
    INT  ET() const { return static_cast<INT>(_ET.count()); }   // elapsed ms

protected:
    BOOL _IN       = false;
    BOOL _Q        = false;
    TIME _PT       = 0ms;
    TIME _ET       = 0ms;
    BOOL _prev_IN  = false;
    BOOL _running  = false;
    std::chrono::time_point<std::chrono::steady_clock> _start{};

    // Template Method: subclasses override only the execution logic.
    virtual void _execute();
};

}  // namespace myplc
