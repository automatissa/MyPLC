#include <iostream>
#include <thread>
#include "../../lib/myplc.h"

int main() {
// Declare value here
myplc::TON ton1;
int8_t STEP_MACHINE = 0;

// Loop Program
while (true) {

    switch (STEP_MACHINE)
    {
    case 0: //init value
        ton1.PT(3s);
        ton1.IN(false);   
        if (!ton1.Q()) {  
            STEP_MACHINE = 1;
            std::cout << "TON Initialized" << std::endl;
            std::cout << "Q = " << ton1.Q() << " and ET = " << ton1.ET() << std::endl;
        }
        break;

    case 1: // Run the TON
        ton1.IN(true);
        if (ton1.Q()) {  
            std::cout << "Tempo executed" << std::endl;
            std::cout << "Q = " << ton1.Q() << " and ET = " << ton1.ET() << std::endl;
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