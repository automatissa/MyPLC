#pragma once

// ============================================================================
//  MyPLC — IEC 61131-3 Function Blocks in C++
//
//  Single include to access all function blocks and standard types.
//
//  Usage:
//    #include "plc/myplc.h"
//    using namespace myplc;
//
//    TON  myTimer;
//    CTU  myCounter;
//
//    void LOOP() {
//        myTimer(start_btn, T(5s));      // same call style as ST
//        if (myTimer.Q()) motor = true;
//    }
// ============================================================================

#include "plc/types.h"

// Timers
#include "plc/timers/TON.h"
#include "plc/timers/TOF.h"
#include "plc/timers/TP.h"

// Edge detectors
#include "plc/triggers/R_TRIG.h"
#include "plc/triggers/F_TRIG.h"

// Counters
#include "plc/counters/CTU.h"
#include "plc/counters/CTD.h"
#include "plc/counters/CTUD.h"

// Bistables (latches)
#include "plc/bistables/SR.h"
#include "plc/bistables/RS.h"
