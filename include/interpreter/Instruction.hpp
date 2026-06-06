#pragma once

#include <string>
#include <sstream>

namespace interpreter {

enum class OpCode {
    LIT,    // Load literal
    LOD,    // Load value from memory address
    STO,    // Store value to memory address
    CAL,    // Call procedure/function - reserved for Faris interpreter integration
    INT,    // Initiate memory
    JMP,    // Unconditional jump
    JPC,    // Conditional jump if false
    OPR,    // Operation
    RET,    // Return / halt
    LODI,   // Load Indirect
    STOI,   // Store Indirect
    READ    // Input
};

enum OprCode {
    OPR_NEG   = 1,
    OPR_ADD   = 2,
    OPR_SUB   = 3,
    OPR_MUL   = 4,
    OPR_DIV   = 5,
    OPR_MOD   = 6,
    OPR_EQL   = 7,
    OPR_NEQ   = 8,
    OPR_LSS   = 9,
    OPR_GEQ   = 10,
    OPR_GTR   = 11,
    OPR_LEQ   = 12,
    OPR_WRT   = 13,
    OPR_WRTLN = 14,
    OPR_AND   = 15, 
    OPR_OR    = 16, 
    OPR_NOT   = 17, 
    OPR_READ  = 18  
};

struct Instruction {
    OpCode op;
    int level;
    int operand;

    Instruction(OpCode opCode, int l = 0, int a = 0)
        : op(opCode), level(l), operand(a) {}


    std::string opName() const {
        switch (op) {
            case OpCode::LIT: return "LIT";
            case OpCode::LOD: return "LOD";
            case OpCode::STO: return "STO";
            case OpCode::CAL: return "CAL";
            case OpCode::INT: return "INT";
            case OpCode::JMP: return "JMP";
            case OpCode::JPC: return "JPC";
            case OpCode::OPR: return "OPR";
            case OpCode::RET: return "RET";
            case OpCode::LODI: return "LODI";
            case OpCode::STOI: return "STOI";
            case OpCode::READ: return "READ";
        }
        return "UNKNOWN";
    }
    std::string toString(int line = -1) const {
        std::ostringstream oss;
        if (line >= 0) oss << line << " ";
        oss << opName() << " " << level << " " << operand;
        return oss.str();
    }
};

} // namespace interpreter
