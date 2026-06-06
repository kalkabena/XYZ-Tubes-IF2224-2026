#include <interpreter/VirtualMachine.hpp>
#include <iostream>

VirtualMachine::VirtualMachine() : instructionPointer(0) {}

void VirtualMachine::loadCode(const std::vector<TACInstruction>& icgCode) {
    code = icgCode;
}

void VirtualMachine::run() {
    instructionPointer = 0;
    memory.reset();

    while (instructionPointer < static_cast<int>(code.size())) {
        TACInstruction instr = code[instructionPointer];
        instructionPointer++;

        if (instr.op == "LIT") {
            // Push literal value onto stack
            memory.push(instr.arg);
        }
        else if (instr.op == "LOD") {
            // Load value from absolute address
            memory.push(memory.getValueAt(instr.arg));
        }
        else if (instr.op == "STO") {
            // Pop top of stack and store at absolute address
            int val = memory.pop();
            memory.setValueAt(instr.arg, val);
        }
        else if (instr.op == "INT") {
            // Allocate space on stack (increment SP by arg)
            memory.allocate(instr.arg);
        }
        else if (instr.op == "JMP") {
            instructionPointer = instr.arg;
        }
        else if (instr.op == "JPC") {
            // Jump if top of stack == 0 (false), then pop
            int cond = memory.pop();
            if (cond == 0) {
                instructionPointer = instr.arg;
            }
        }
        else if (instr.op == "RET") {
            if (memory.getBasePointer() == 0) return;
        }
        else if (instr.op == "LODI") {
            // Indirect load: pop address, push value at that address
            int addr = memory.pop();
            memory.push(memory.getValueAt(addr));
        }
        else if (instr.op == "STOI") {
            // Indirect store: pop value, pop address, store value at address
            int val  = memory.pop();
            int addr = memory.pop();
            memory.setValueAt(addr, val);
        }
        else if (instr.op == "OPR") {
            int val1, val2;
            switch (instr.arg) {
                case 1: // Unary negation
                    val1 = memory.pop();
                    memory.push(-val1);
                    break;
                case 2: // ADD
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 + val2);
                    break;
                case 3: // SUB
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 - val2);
                    break;
                case 4: // MUL
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 * val2);
                    break;
                case 5: // DIV
                    val2 = memory.pop();
                    val1 = memory.pop();
                    if (val2 == 0) throw std::runtime_error("Runtime Error: Division by zero.");
                    memory.push(val1 / val2);
                    break;
                case 6: // MOD
                    val2 = memory.pop();
                    val1 = memory.pop();
                    if (val2 == 0) throw std::runtime_error("Runtime Error: Modulus by zero.");
                    memory.push(val1 % val2);
                    break;
                case 7: // EQL
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 == val2 ? 1 : 0);
                    break;
                case 8: // NEQ
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 != val2 ? 1 : 0);
                    break;
                case 9: // LSS
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 < val2 ? 1 : 0);
                    break;
                case 10: // GEQ
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 >= val2 ? 1 : 0);
                    break;
                case 11: // GTR
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 > val2 ? 1 : 0);
                    break;
                case 12: // LEQ
                    val2 = memory.pop();
                    val1 = memory.pop();
                    memory.push(val1 <= val2 ? 1 : 0);
                    break;
                case 13: // WRITE (no newline)
                    std::cout << memory.pop();
                    break;
                case 14: // WRITELN
                    std::cout << memory.pop() << std::endl;
                    break;
                case 15: // READ
                {
                    int inputVal;
                    std::cin >> inputVal;
                    memory.push(inputVal);
                    break;
                }
                default:
                    throw std::runtime_error("Runtime Error: Unknown OPR code " + std::to_string(instr.arg) + ".");
            }
        }
        else {
            throw std::runtime_error("Runtime Error: Unknown instruction op: " + instr.op);
        }
    }
}
