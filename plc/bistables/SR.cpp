#include "plc/bistables/SR.h"

namespace myplc {

void SR::operator()(BOOL s1, BOOL r) {
    _Q1 = s1 || (_Q1 && !r);   // SET (s1) is dominant
}

}  // namespace myplc
