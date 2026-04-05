#include "plc/bistables/RS.h"

namespace myplc {

void RS::operator()(BOOL s, BOOL r1) {
    _Q1 = (s || _Q1) && !r1;   // RESET (r1) is dominant
}

}  // namespace myplc
