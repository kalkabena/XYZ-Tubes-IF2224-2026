#include "interpreter/StackMemory.hpp"
#include <stdexcept>

StackMemory::StackMemory() {
    memory.resize(MAX_STACK_SIZE, 0);
    reset();
}

void StackMemory::reset() {
    stackPointer = -1;
    basePointer = 0;
    std::fill(memory.begin(), memory.end(), 0);
}

void StackMemory::checkOverflow(int increment) const {
    if (stackPointer + increment >= MAX_STACK_SIZE) {
        throw std::runtime_error("Runtime Error: Stack Overflow. Batas alokasi memori runtime Arion terlampaui.");
    }
}

void StackMemory::checkUnderflow(int decrement) const {
    if (stackPointer - decrement < -1) {
        throw std::runtime_error("Runtime Error: Stack Underflow. Upaya pemanggilan data ilegal pada stack kosong.");
    }
}

void StackMemory::checkBounds(int address) const {
    if (address < 0 || address >= MAX_STACK_SIZE) {
        throw std::runtime_error("Runtime Error: Out-of-Bounds Memory Access. Alamat indeks memori berada di luar jangkauan.");
    }
}

void StackMemory::push(int value) {
    checkOverflow(1);
    stackPointer++;
    memory[stackPointer] = value;
}

int StackMemory::pop() {
    checkUnderflow(1);
    int val = memory[stackPointer];
    stackPointer--;
    return val;
}

int StackMemory::top() const {
    checkUnderflow(0);
    return memory[stackPointer];
}

void StackMemory::allocate(int size) {
    checkOverflow(size);
    stackPointer += size;
}

void StackMemory::deallocate(int size) {
    checkUnderflow(size);
    stackPointer -= size;
}

int StackMemory::getValueAt(int address) const {
    checkBounds(address);
    return memory[address];
}

void StackMemory::setValueAt(int address, int value) {
    checkBounds(address);
    memory[address] = value;
}