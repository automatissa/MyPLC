// ============================================================================
//  Unity build — includes all plc/ library sources for PlatformIO.
//  PlatformIO only compiles files inside src/, so we pull in the shared
//  library via this single translation unit.
// ============================================================================

// Timers
#include "plc/timers/TON.cpp"
#include "plc/timers/TOF.cpp"
#include "plc/timers/TP.cpp"

// Triggers
#include "plc/triggers/R_TRIG.cpp"
#include "plc/triggers/F_TRIG.cpp"

// Counters
#include "plc/counters/CTU.cpp"
#include "plc/counters/CTD.cpp"
#include "plc/counters/CTUD.cpp"

// Bistables
#include "plc/bistables/SR.cpp"
#include "plc/bistables/RS.cpp"

// Registry (Modbus address mapping)
#include "sim/registry.cpp"
