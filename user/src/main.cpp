#include <iostream>
#include <thread>
#include "../../src/myplc.h"

int main() {
// Declare value here
int8_t STEP_MACHINE = 0;
myplc::CTU CTU1;
bool continuing, inputCU;

// Loop Program
while (true) {

    switch (STEP_MACHINE)
    {
    case 0: //init
        std::cout<< "Starting : CV = " << CTU1.CV() << " Q = " << CTU1.Q() << std::endl;
        CTU1.RESET(true);
        std::cout<< "Reset : CV = " << CTU1.CV() << " Q = " << CTU1.Q() << std::endl;
        CTU1.PV(3);
        STEP_MACHINE =1;
        break;

    case 1: //
        std::cout<< " CU Value "<< std::endl;
        std::cin >> inputCU ;
        CTU1.CU(inputCU);
        std::cout<< "CV = " << CTU1.CV() << " Q = " << CTU1.Q() << std::endl;
        std::cout<< "Tap 1 to continue or 0 to reset " << std::endl;
        std::cin >> continuing ;
        if (continuing) {
        STEP_MACHINE = 1;
        }
        else {
        STEP_MACHINE = 0;
        }
        break;
    
    default:
        break;
    }

    std::this_thread::sleep_for(10ms); // 10ms cycle
}
return 0;
}

