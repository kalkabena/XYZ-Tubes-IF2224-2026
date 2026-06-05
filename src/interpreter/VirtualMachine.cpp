#include <interpreter/VirtualMachine.hpp>
#include <iostream>

VirtualMachine::VirtualMachine() {
    instructionPointer = 0;
    basePointer = 0;
    stackPointer = -1;
    stack.resize(MAX_STACK_SIZE, 0);
}

void VirtualMachine::loadCode(const std::vector<TACInstruction>& icgCode) {
    code = icgCode;
}

void VirtualMachine::checkStackOverflow(int increment) {
    if (stackPointer + increment >= MAX_STACK_SIZE) {
        throw std::runtime_error("Runtime Error: Stack Overflow.");
    }
}

void VirtualMachine::checkStackUnderflow(int decrement) {
    if (stackPointer - decrement < -1) {
        throw std::runtime_error("Runtime Error: Stack Underflow.");
    }
}

void VirtualMachine::checkMemoryBounds(int address) {
    if (address < 0 || address >= MAX_STACK_SIZE) {
        throw std::runtime_error("Runtime Error: Out-of-Bounds Memory Access.");
    }
}

void VirtualMachine::run() {
    instructionPointer = 0;
    
    while (instructionPointer < static_cast<int>(code.size())) {
        TACInstruction instr = code[instructionPointer];
        instructionPointer++;
        
        if (instr.op == "LIT") {
            checkStackOverflow(1);
            stackPointer++;
            stack[stackPointer] = instr.arg;
        } 
        else if (instr.op == "LOD") {
            checkStackOverflow(1);
            checkMemoryBounds(instr.arg);
            stackPointer++;
            stack[stackPointer] = stack[instr.arg];
        } 
        else if (instr.op == "STO") {
            checkStackUnderflow(1);
            checkMemoryBounds(instr.arg);
            stack[instr.arg] = stack[stackPointer];
            stackPointer--;
        } 
        else if (instr.op == "INT") {
            checkStackOverflow(instr.arg);
            stackPointer += instr.arg;
        } 
        else if (instr.op == "JMP") {
            instructionPointer = instr.arg;
        } 
        else if (instr.op == "JPC") {
            checkStackUnderflow(1);
            if (stack[stackPointer] == 0) {
                instructionPointer = instr.arg;
            }
            stackPointer--;
        } 
        else if (instr.op == "RET") {
            if (basePointer == 0) return;
        } 
        else if (instr.op == "LODI") {
            checkStackUnderflow(1);
            int addr = stack[stackPointer];
            checkMemoryBounds(addr);
            stack[stackPointer] = stack[addr]; 
        } 
        else if (instr.op == "STOI") {
            checkStackUnderflow(2);
            int val = stack[stackPointer];
            int addr = stack[stackPointer - 1];
            checkMemoryBounds(addr);
            stack[addr] = val;
            stackPointer -= 2;
        }
        else if (instr.op == "OPR") {
            int val1, val2;
            switch(instr.arg) {
                case 1:
                    checkStackUnderflow(0);
                    stack[stackPointer] = -stack[stackPointer];
                    break;
                case 2: // ADD
                case 3: // SUB
                case 4: // MUL
                case 5: // DIV
                case 6: // MOD
                case 7: // EQL
                case 8: // NEQ
                case 9: // LSS
                case 10: // GEQ
                case 11: // GTR
                case 12: // LEQ
                    checkStackUnderflow(2);
                    val2 = stack[stackPointer--];
                    val1 = stack[stackPointer];
                    if (instr.arg == 2) stack[stackPointer] = val1 + val2;
                    else if (instr.arg == 3) stack[stackPointer] = val1 - val2;
                    else if (instr.arg == 4) stack[stackPointer] = val1 * val2;
                    else if (instr.arg == 5) {
                        if (val2 == 0) throw std::runtime_error("Runtime Error: Division by zero.");
                        stack[stackPointer] = val1 / val2;
                    }
                    else if (instr.arg == 6) {
                        if (val2 == 0) throw std::runtime_error("Runtime Error: Modulus by zero.");
                        stack[stackPointer] = val1 % val2;
                    }
                    else if (instr.arg == 7) stack[stackPointer] = (val1 == val2);
                    else if (instr.arg == 8) stack[stackPointer] = (val1 != val2);
                    else if (instr.arg == 9) stack[stackPointer] = (val1 < val2);
                    else if (instr.arg == 10) stack[stackPointer] = (val1 >= val2);
                    else if (instr.arg == 11) stack[stackPointer] = (val1 > val2);
                    else if (instr.arg == 12) stack[stackPointer] = (val1 <= val2);
                    break;
                case 13:
                    checkStackUnderflow(1);
                    std::cout << stack[stackPointer];
                    stackPointer--;
                    break;
                case 14:
                    checkStackUnderflow(1);
                    std::cout << stack[stackPointer] << std::endl;
                    stackPointer--;
                    break;
                case 15: 
                    checkStackOverflow(1);
                    stack[++stackPointer] = 15;
                    break;
                default:
                    throw std::runtime_error("Runtime Error: Unknown OPR code.");
            }
        } 
        else {
            throw std::runtime_error("Runtime Error: Unknown Instruction OP: " + instr.op);
        }
    }
}