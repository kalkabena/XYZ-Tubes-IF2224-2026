#pragma once

#include <bits/stdc++.h>
#include "icg/TACInstruction.hpp"


class VirtualMachine {
private:
    static const int MAX_STACK_SIZE = 1000;
    std::vector<TACInstruction> code; 
    std::vector<int> stack;           
    
    int instructionPointer;
    int basePointer;       
    int stackPointer;      

    void checkStackOverflow(int increment);
    void checkStackUnderflow(int decrement);
    void checkMemoryBounds(int address);

public:
    VirtualMachine();

    void loadCode(const std::vector<TACInstruction>& icgCode);
    
    void run();
};