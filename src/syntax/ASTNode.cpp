#include "syntax/ASTNode.hpp"
#include <stdexcept>
using namespace std;
void AssignNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "Assign --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    if (targetVariable) {
        targetVariable->print(os, childPrefix, false);
    } else {
        os << childPrefix << "├── [NULL TARGET]\n";
    }
    
    if (value) {
        value->print(os, childPrefix, true);
    } else {
        os << childPrefix << "└── [NULL VALUE]\n";
    }
}

void CallNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "CallNode(func: '" << functionName << "') --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (arguments[i]) arguments[i]->print(os, childPrefix, i == arguments.size() - 1);
        else os << childPrefix << (i == arguments.size() - 1 ? "└── " : "├── ") << "[NULL ARGUMENT]\n";
    }
}

void BinOpNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "BinaryExpr: '" << op << "' --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (left) left->print(os, childPrefix, false);
    else os << childPrefix << "├── [NULL LEFT]\n";
    if (right) right->print(os, childPrefix, true);
    else os << childPrefix << "└── [NULL RIGHT]\n";
}

void IfNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "IfStatement --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    os << childPrefix << "├── Condition:\n";
    if (condition) condition->print(os, childPrefix + "│   ", true);
    else os << childPrefix << "│   └── [NULL CONDITION]\n";
    
    os << childPrefix << (elseBranch ? "├── Then:\n" : "└── Then:\n");
    if (thenBranch) thenBranch->print(os, childPrefix + (elseBranch ? "│   " : "    "), true);
    else os << childPrefix << (elseBranch ? "│   " : "    ") << "└── [NULL THEN]\n";
    
    if (elseBranch) {
        os << childPrefix << "└── Else:\n";
        elseBranch->print(os, childPrefix + "    ", true);
    }
}

void WhileNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "WhileStatement --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    os << childPrefix << "├── Condition:\n";
    if (condition) condition->print(os, childPrefix + "│   ", true);
    else os << childPrefix << "│   └── [NULL CONDITION]\n";
    
    os << childPrefix << "└── Body:\n";
    if (body) body->print(os, childPrefix + "    ", true);
    else os << childPrefix << "    └── [NULL BODY]\n";
}

void RepeatNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "RepeatStatement --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    os << childPrefix << "├── Body:\n";
    if (body) body->print(os, childPrefix + "│   ", true);
    else os << childPrefix << "│   └── [NULL BODY]\n";
    
    os << childPrefix << "└── Condition:\n";
    if (condition) condition->print(os, childPrefix + "    ", true);
    else os << childPrefix << "    └── [NULL CONDITION]\n";
}

void ForNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "ForStatement(var: " << varName << ") --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    os << childPrefix << "├── Start:\n";
    if (start) start->print(os, childPrefix + "│   ", true);
    else os << childPrefix << "│   └── [NULL START]\n";
    
    os << childPrefix << "├── End:\n";
    if (end) end->print(os, childPrefix + "│   ", true);
    else os << childPrefix << "│   └── [NULL END]\n";
    
    os << childPrefix << "└── Body:\n";
    if (body) body->print(os, childPrefix + "    ", true);
    else os << childPrefix << "    └── [NULL BODY]\n";
}

void CaseNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "CaseStatement --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    os << childPrefix << (branches.empty() ? "└── Expression:\n" : "├── Expression:\n");
    if (expression) expression->print(os, childPrefix + (branches.empty() ? "    " : "│   "), true);
    else os << childPrefix << (branches.empty() ? "    " : "│   ") << "└── [NULL EXPRESSION]\n";
    
    for (size_t i = 0; i < branches.size(); ++i) {
        bool lastBranch = (i == branches.size() - 1);
        os << childPrefix << (lastBranch ? "└── " : "├── ") << "Branch: " << branches[i].first << "\n";
        
        if (branches[i].second) branches[i].second->print(os, childPrefix + (lastBranch ? "    " : "│   "), true);
        else os << childPrefix + (lastBranch ? "    " : "│   ") << "└── [NULL STATEMENT]\n";
    }
}

void VarDeclNode::print(ostream& os, string prefix, bool isLast) const {
    for (size_t i = 0; i < varNames.size(); ++i) {
        os << prefix << (isLast && i == varNames.size() - 1 ? "└── " : "├── ") 
           << "VarDecl: " << varNames[i] << " : " << type 
           << " --> [Type: " << type << " | Scope: " << scopeLevel << "]\n";
    }
}

void SubprogramDeclNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "SubprogramDecl: " << name << " --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    if (block) block->print(os, prefix + (isLast ? "    " : "│   "), true);
    else os << prefix + (isLast ? "    " : "│   ") << "└── [NULL BLOCK]\n";
}

void FieldAccessNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") 
       << "FieldAccess: " << recordName << "." << fieldName << " --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
}

void ArrayAccessNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") 
       << "ArrayAccess(array: " << arrayName << ") --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
       
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (indexExpression) indexExpression->print(os, childPrefix, true); 
    else os << childPrefix << "└── [NULL INDEX]\n";
}

void UnaryOpNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "UnaryExpr: '" << op << "' --> [Type: " << exprType << " | Scope: " << scopeLevel << "]\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (operand) operand->print(os, childPrefix, true);
    else os << childPrefix << "└── [NULL OPERAND]\n";
}