#include "ASTNode.hpp"
#include <stdexcept>
using namespace std;
void AssignNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "AssignNode\n";
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
    os << prefix << (isLast ? "└── " : "├── ") << "CallNode(func: '" << functionName << "')\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (arguments[i]) arguments[i]->print(os, childPrefix, i == arguments.size() - 1);
        else os << childPrefix << (i == arguments.size() - 1 ? "└── " : "├── ") << "[NULL ARGUMENT]\n";
    }
}

void BinOpNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "BinOpNode(op: '" << op << "')\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    if (left) left->print(os, childPrefix, false);
    else os << childPrefix << "├── [NULL LEFT]\n";
    if (right) right->print(os, childPrefix, true);
    else os << childPrefix << "└── [NULL RIGHT]\n";
}

void IfNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "IfNode\n";
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
    os << prefix << (isLast ? "└── " : "├── ") << "WhileNode\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    if (condition) condition->print(os, childPrefix + "├── ", false);
    else os << childPrefix << "├── [NULL CONDITION]\n";
    
    if (body) body->print(os, childPrefix + "└── ", true);
    else os << childPrefix << "└── [NULL BODY]\n";
}

void RepeatNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "RepeatNode\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    if (body) body->print(os, childPrefix + "├── ", false);
    else os << childPrefix << "├── [NULL BODY]\n";
    
    if (condition) condition->print(os, childPrefix + "└── ", true);
    else os << childPrefix << "└── [NULL CONDITION]\n";
}

void ForNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "ForNode(var: " << varName << ")\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    if (start) start->print(os, childPrefix + "├── ", false);
    else os << childPrefix << "├── [NULL START]\n";
    
    if (end) end->print(os, childPrefix + "├── ", false);
    else os << childPrefix << "├── [NULL END]\n";
    
    if (body) body->print(os, childPrefix + "└── ", true);
    else os << childPrefix << "└── [NULL BODY]\n";
}

void CaseNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "CaseNode\n";
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    if (expression) expression->print(os, childPrefix + "├── ", branches.empty());
    else os << childPrefix << (branches.empty() ? "└── " : "├── ") << "[NULL EXPRESSION]\n";
    
    for (size_t i = 0; i < branches.size(); ++i) {
        bool lastBranch = (i == branches.size() - 1);
        os << childPrefix << (lastBranch ? "└── " : "├── ") << "Branch: " << branches[i].first << "\n";
        
        if (branches[i].second) branches[i].second->print(os, childPrefix + (lastBranch ? "    " : "│   "), true);
        else os << childPrefix + (lastBranch ? "    " : "│   ") << "└── [NULL STATEMENT]\n";
    }
}

void VarDeclNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "VarDeclNode(Type: " << type << ")\n";
    for (size_t i = 0; i < varNames.size(); ++i) {
        os << prefix << (isLast ? "    " : "│   ") 
           << (i == varNames.size() - 1 ? "└── " : "├── ") << varNames[i] << "\n";
    }
}

void SubprogramDeclNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") << "SubprogramDecl(" << name << ")\n";
    if (block) block->print(os, prefix + (isLast ? "    " : "│   "), true);
    else os << prefix + (isLast ? "    " : "│   ") << "└── [NULL BLOCK]\n";
}

void FieldAccessNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") 
       << "FieldAccessNode(" << recordName << "." << fieldName << ")\n";
}

void ArrayAccessNode::print(ostream& os, string prefix, bool isLast) const {
    os << prefix << (isLast ? "└── " : "├── ") 
       << "ArrayAccessNode(array: " << arrayName << ")\n";
       
    string childPrefix = prefix + (isLast ? "    " : "│   ");
    
    if (indexExpression) indexExpression->print(os, childPrefix, true); 
    else os << childPrefix << "└── [NULL INDEX]\n";
}