#include "interpreter/SafeStackMachine.hpp"
#include "interpreter/RuntimeProtection.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace interpreter {

SafeStackMachine::SafeStackMachine(std::vector<Instruction> instructions)
    : code(std::move(instructions)), output(&std::cout) {}

void SafeStackMachine::setOutputStream(std::ostream& os) {
    output = &os;
}

void SafeStackMachine::reset() {
    stack.clear();
    memory.clear();
    ip = 0;
    halted = false;
}

const std::vector<int>& SafeStackMachine::getStack() const {
    return stack;
}

const std::vector<int>& SafeStackMachine::getMemory() const {
    return memory;
}

std::size_t SafeStackMachine::getInstructionPointer() const {
    return ip;
}

void SafeStackMachine::push(int value) {
    RuntimeProtection::checkStackOverflow(stack.size());
    stack.push_back(value);
}

int SafeStackMachine::pop() {
    RuntimeProtection::checkStackUnderflow(stack.size());
    int value = stack.back();
    stack.pop_back();
    return value;
}

void SafeStackMachine::run() {
    while (!halted) {
        RuntimeProtection::checkInstructionPointer(ip, code.size());
        const Instruction instruction = code[ip];

        switch (instruction.op) {
            case OpCode::INT: {
                if (instruction.operand < 0) {
                    throw RuntimeProtectionError("MemoryAllocationError: ukuran memory tidak boleh negatif");
                }
                memory.assign(static_cast<std::size_t>(instruction.operand), 0);
                ++ip;
                break;
            }

            case OpCode::LIT: {
                push(instruction.operand);
                ++ip;
                break;
            }

            case OpCode::LOD: {
                RuntimeProtection::checkMemoryAddress(instruction.operand, memory.size(), "LOD");
                push(memory[static_cast<std::size_t>(instruction.operand)]);
                ++ip;
                break;
            }

            case OpCode::STO: {
                RuntimeProtection::checkMemoryAddress(instruction.operand, memory.size(), "STO");
                memory[static_cast<std::size_t>(instruction.operand)] = pop();
                ++ip;
                break;
            }

            case OpCode::JMP: {
                RuntimeProtection::checkJumpTarget(instruction.operand, code.size());
                ip = static_cast<std::size_t>(instruction.operand);
                break;
            }

            case OpCode::JPC: {
                int condition = pop();
                if (condition == 0) {
                    RuntimeProtection::checkJumpTarget(instruction.operand, code.size());
                    ip = static_cast<std::size_t>(instruction.operand);
                } else {
                    ++ip;
                }
                break;
            }

            case OpCode::OPR: {
                executeOperation(instruction.operand);
                ++ip;
                break;
            }

            case OpCode::RET: {
                halted = true;
                break;
            }

            case OpCode::CAL: {
                RuntimeProtection::checkCallDepth(0);
                throw RuntimeProtectionError(
                    "UnsupportedInstructionError: CAL belum diaktifkan pada SafeStackMachine. "
                    "Gunakan modul ini sebagai proteksi runtime, lalu sambungkan ke interpreter fungsi/prosedur Faris."
                );
            }
        }
    }
}

void SafeStackMachine::executeOperation(int operationCode) {
    switch (operationCode) {
        case OPR_NEG: {
            int value = pop();
            push(RuntimeProtection::safeNeg(value));
            break;
        }
        case OPR_ADD: {
            int right = pop();
            int left = pop();
            push(RuntimeProtection::safeAdd(left, right));
            break;
        }
        case OPR_SUB: {
            int right = pop();
            int left = pop();
            push(RuntimeProtection::safeSub(left, right));
            break;
        }
        case OPR_MUL: {
            int right = pop();
            int left = pop();
            push(RuntimeProtection::safeMul(left, right));
            break;
        }
        case OPR_DIV: {
            int right = pop();
            int left = pop();
            push(RuntimeProtection::safeDiv(left, right));
            break;
        }
        case OPR_MOD: {
            int right = pop();
            int left = pop();
            push(RuntimeProtection::safeMod(left, right));
            break;
        }
        case OPR_EQL: {
            int right = pop();
            int left = pop();
            push(left == right ? 1 : 0);
            break;
        }
        case OPR_NEQ: {
            int right = pop();
            int left = pop();
            push(left != right ? 1 : 0);
            break;
        }
        case OPR_LSS: {
            int right = pop();
            int left = pop();
            push(left < right ? 1 : 0);
            break;
        }
        case OPR_GEQ: {
            int right = pop();
            int left = pop();
            push(left >= right ? 1 : 0);
            break;
        }
        case OPR_GTR: {
            int right = pop();
            int left = pop();
            push(left > right ? 1 : 0);
            break;
        }
        case OPR_LEQ: {
            int right = pop();
            int left = pop();
            push(left <= right ? 1 : 0);
            break;
        }
        case OPR_WRT: {
            int value = pop();
            (*output) << value;
            break;
        }
        case OPR_WRTLN: {
            int value = pop();
            (*output) << value << '\n';
            break;
        }
        default:
            throw RuntimeProtectionError(
                "InvalidOperationError: kode OPR " + std::to_string(operationCode) + " tidak dikenali"
            );
    }
}

} // namespace interpreter
