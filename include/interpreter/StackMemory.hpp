#pragma once
#include <vector>
#include <string>

class StackMemory {
private:
    static const int MAX_STACK_SIZE = 1000;
    std::vector<int> memory;
    int stackPointer;
    int basePointer;

public:
    StackMemory();
    ~StackMemory() = default;

    void push(int value);
    int pop();
    int top() const;

    void allocate(int size);
    void deallocate(int size);

    int getValueAt(int address) const;
    void setValueAt(int address, int value);

    int getStackPointer() const { return stackPointer; }
    int getBasePointer() const { return basePointer; }
    void setBasePointer(int bp) { basePointer = bp; }
    void adjustStackPointer(int offset) { stackPointer += offset; }

    void reset();

    void checkOverflow(int increment) const;
    void checkUnderflow(int decrement) const;
    void checkBounds(int address) const;
};