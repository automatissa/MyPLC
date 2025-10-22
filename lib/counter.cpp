#include "myplc.h"

namespace myplc {

    void CTU::CU(bool init_CU) {
        _CU = init_CU;
        if (_CU && !old_CU) {
            old_CU = true;
            _CV++;
            if (_CV >= _PV) {
            _Q = true;
            } else {
            _Q = false;
            }
        }

        if( !_CU && old_CU) {
            old_CU = false;
        }
        

    }

    void CTU::RESET(bool init_RESET) {
        _RESET = init_RESET ;
        if (_RESET){
            _CV = 0;
            _PV = 0;
            _CU = false;
            old_CU = false;
            _RESET = false;
            _Q = false;
        }
    }
    
}