#pragma once

#include "interpreter/Instruction.hpp"
#include <cstddef>
#include <iosfwd>
#include <vector>

namespace interpreter {

class SafeStackMachine {
private:
    std::vector<Instruction> code;
    std::vector<int> stack;
    std::vector<int> memory;
    std::size_t ip = 0;
    bool halted = false;
    std::ostream* output = nullptr;

    int pop();
    void push(int value);
    void executeOperation(int operationCode);

public:
    explicit SafeStackMachine(std::vector<Instruction> instructions);

    void setOutputStream(std::ostream& os);
    void run();
    void reset();

    const std::vector<int>& getStack() const;
    const std::vector<int>& getMemory() const;
    std::size_t getInstructionPointer() const;
};

} // namespace interpreter
