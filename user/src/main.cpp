#include <iostream>
#include <thread>
#include "myplc.h"

int main() {
// Declare value here
int STEP_MACHINE = 0;

// Loop Program
while (true) {

    switch (STEP_MACHINE)
    {
    case 0: //init
        std::cout<<"Hello myplc"<<std::endl;
        STEP_MACHINE =1;
        break;

    case 1: //

        break;
    
    default:
        break;
    }

    std::this_thread::sleep_for(10ms); // 10ms cycle
}
return 0;
}

