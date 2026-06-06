#pragma once

#include <vector>
#include <string>
#include "icg/TACInstruction.hpp"
#include "interpreter/StackMemory.hpp"


class VirtualMachine {
private:
    std::vector<TACInstruction> code;
    StackMemory memory;
    int instructionPointer;

public:
    VirtualMachine();

    void loadCode(const std::vector<TACInstruction>& icgCode);
    void run();
};