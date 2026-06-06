#include <icg/ICGenerator.hpp>
#include <syntax/ASTNode.hpp>
#include <semantic/SymbolTable.hpp>
#include <bits/stdc++.h>

using namespace std;

static int extractIntSafe(const string& val) {
    string raw = val;
    size_t start = raw.find('(');
    size_t end = raw.rfind(')');
    if (start != string::npos && end != string::npos && start < end) {
        raw = raw.substr(start + 1, end - start - 1);
    }
    try {
        return stoi(raw);
    } catch (...) {
        return 0;
    }
}

int ICGenerator::emit(const string& op, int level, int arg) {
    code.push_back({currentLine, op, level, arg});
    return currentLine++;
}

void ICGenerator::backpatch(int lineIndex, int targetLine) {
    code[lineIndex].arg = targetLine;
}

void ICGenerator::printCode() const {
    for (const auto& instr : code) {
        cout << instr.lineNo << " " << instr.op << " " << instr.level << " " << instr.arg << "\n";
    }
}

void ICGenerator::generate(ASTNode* node) {
    emit("INT", 0, 1000);
    if (node) node->accept(this);
    emit("RET", 0, 0);
}

void ICGenerator::visit(BlockNode* node) {
    for (auto& stmt : node->statements) {
        if (stmt) stmt->accept(this);
    }
}

void ICGenerator::visit(NumberNode* node) {
    emit("LIT", 0, extractIntSafe(node->value));
}

void ICGenerator::visit(StringNode*) {}

void ICGenerator::visit(VariableNode* node) {
    int idx = symbolTable.lookupIndex(node->name);
    if (idx > 0) {
        emit("LOD", 0, symbolTable.getTabEntry(idx).adr);
    }
}

void ICGenerator::visit(AssignNode* node) {
    if (node->value) node->value->accept(this);
    
    if (auto varTarget = dynamic_cast<VariableNode*>(node->targetVariable.get())) {
        int idx = symbolTable.lookupIndex(varTarget->name);
        if (idx > 0) emit("STO", 0, symbolTable.getTabEntry(idx).adr);
    } 
    else if (auto arrTarget = dynamic_cast<ArrayAccessNode*>(node->targetVariable.get())) {
        int idx = symbolTable.lookupIndex(arrTarget->arrayName);
        if (idx > 0) {
            auto tabEntry = symbolTable.getTabEntry(idx);
            auto atabEntry = symbolTable.getAtabEntry(tabEntry.ref);
            emit("LIT", 0, tabEntry.adr);
            if (arrTarget->indexExpression) arrTarget->indexExpression->accept(this);
            emit("LIT", 0, atabEntry.low);
            emit("OPR", 0, 3);
            emit("LIT", 0, atabEntry.elsz);
            emit("OPR", 0, 4);
            emit("OPR", 0, 2);
            emit("STOI", 0, 0);
        }
    }
}

void ICGenerator::visit(UnaryOpNode* node) {
    if (node->operand) {
        node->operand->accept(this);
    }

    if (node->op == "-" || node->op == "minus") {
        emit("OPR", 0, 1);
    } 
    else if (node->op == "not") {
        emit("LIT", 0, 1);
        emit("OPR", 0, 3); 
    }
}

void ICGenerator::visit(BinOpNode* node) {
    if (node->left) node->left->accept(this);
    if (node->right) node->right->accept(this);

    if (node->op == "+") emit("OPR", 0, 2);
    else if (node->op == "-") emit("OPR", 0, 3);
    else if (node->op == "*") emit("OPR", 0, 4);
    else if (node->op == "div" || node->op == "/" || node->op == "idiv") emit("OPR", 0, 5);
    else if (node->op == "MOD" || node->op == "imod") emit("OPR", 0, 6);
    else if (node->op == "==") emit("OPR", 0, 7);
    else if (node->op == "<>") emit("OPR", 0, 8);
    else if (node->op == "<") emit("OPR", 0, 9);
    else if (node->op == ">=") emit("OPR", 0, 10);
    else if (node->op == ">") emit("OPR", 0, 11);
    else if (node->op == "<=") emit("OPR", 0, 12);
}

void ICGenerator::visit(IfNode* node) {
    if (node->condition) node->condition->accept(this);
    int jpcIdx = emit("JPC", 0, -1);
    if (node->thenBranch) node->thenBranch->accept(this);
    if (node->elseBranch) {
        int jmpIdx = emit("JMP", 0, -1);
        backpatch(jpcIdx, currentLine);
        node->elseBranch->accept(this);
        backpatch(jmpIdx, currentLine);
    } else {
        backpatch(jpcIdx, currentLine);
    }
}

void ICGenerator::visit(WhileNode* node) {
    int startLine = currentLine;
    if (node->condition) node->condition->accept(this);
    int jpcIdx = emit("JPC", 0, -1);
    if (node->body) node->body->accept(this);
    emit("JMP", 0, startLine);
    backpatch(jpcIdx, currentLine);
}

void ICGenerator::visit(CallNode* node) {
    if (node->functionName == "writeln" || node->functionName == "write") {
        for (size_t i = 0; i < node->arguments.size(); ++i) {
            auto& arg = node->arguments[i];
            if (arg) {
                if (dynamic_cast<StringNode*>(arg.get())) {
                    continue; // Skip string node because VM doesn't support strings
                }
                arg->accept(this);
                
                bool isLastNonString = true;
                for (size_t j = i + 1; j < node->arguments.size(); ++j) {
                    if (node->arguments[j] && !dynamic_cast<StringNode*>(node->arguments[j].get())) {
                        isLastNonString = false;
                        break;
                    }
                }
                
                if (node->functionName == "writeln" && isLastNonString) {
                    emit("OPR", 0, 14);
                } else {
                    emit("OPR", 0, 13);
                }
            }
        }
    } else {
        emit("LIT", 0, 0);
    }
}

void ICGenerator::visit(RepeatNode* node) {
    int startLine = currentLine;
    if (node->body) node->body->accept(this);
    if (node->condition) node->condition->accept(this);
    emit("JPC", 0, startLine);
}

void ICGenerator::visit(ForNode* node) {
    if (node->start) node->start->accept(this);
    int varIdx = symbolTable.lookupIndex(node->varName);
    int varAddress = (varIdx > 0) ? symbolTable.getTabEntry(varIdx).adr : 0;
    emit("STO", 0, varAddress);
    int loopStart = currentLine;
    emit("LOD", 0, varAddress);
    if (node->end) node->end->accept(this);
    emit("OPR", 0, 12);
    int jpcIdx = emit("JPC", 0, -1);
    if (node->body) node->body->accept(this);
    emit("LOD", 0, varAddress);
    emit("LIT", 0, 1);
    emit("OPR", 0, 2);
    emit("STO", 0, varAddress);
    emit("JMP", 0, loopStart);
    backpatch(jpcIdx, currentLine);
}

void ICGenerator::visit(CaseNode* node) {
    vector<int> endJumps;
    for (auto& branch : node->branches) {
        if (node->expression) node->expression->accept(this);
        emit("LIT", 0, extractIntSafe(branch.first));
        emit("OPR", 0, 7);
        int jpcIdx = emit("JPC", 0, -1);
        if (branch.second) branch.second->accept(this);
        endJumps.push_back(emit("JMP", 0, -1));
        backpatch(jpcIdx, currentLine);
    }
    for (int jmpIdx : endJumps) backpatch(jmpIdx, currentLine);
}

void ICGenerator::visit(VarDeclNode*) {}

void ICGenerator::visit(SubprogramDeclNode* node) {
    int skipJump = emit("JMP", 0, -1);
    emit("INT", 0, 5 + 10);
    if (node->block) node->block->accept(this);
    emit("RET", 0, 0);
    backpatch(skipJump, currentLine);
}

void ICGenerator::visit(FieldAccessNode* node) {
    int idx = symbolTable.lookupIndex(node->recordName);
    if (idx > 0) emit("LOD", 0, symbolTable.getTabEntry(idx).adr);
}

void ICGenerator::visit(ArrayAccessNode* node) {
    int idx = symbolTable.lookupIndex(node->arrayName);
    if (idx > 0) {
        auto tabEntry = symbolTable.getTabEntry(idx);
        auto atabEntry = symbolTable.getAtabEntry(tabEntry.ref);
        emit("LIT", 0, tabEntry.adr);
        if (node->indexExpression) node->indexExpression->accept(this);
        emit("LIT", 0, atabEntry.low);
        emit("OPR", 0, 3);
        emit("LIT", 0, atabEntry.elsz);
        emit("OPR", 0, 4);
        emit("OPR", 0, 2);
        emit("LODI", 0, 0);
    }
}