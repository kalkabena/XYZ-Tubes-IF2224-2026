#include "Node.hpp"

void Node::addChild(unique_ptr<Node> child) {
    children.push_back(std::move(child));
}

void Node::printDFS(std::ostream& os, std::string prefix = "", bool isLast = true, bool isRoot = true) const {
    if (!isRoot) {
        os << prefix;
        os << (isLast ? "└── " : "├── ");
    }

    os << nodeTypeToString(type);
    if (!lexeme.empty()) {
        os << "(" << lexeme << ")";
    }
    os << "\n";

    std::string childPrefix = prefix;
    if (!isRoot) {
        childPrefix += (isLast ? "    " : "│   ");
    }

    for (size_t i = 0; i < children.size(); ++i) {
        bool lastChild = (i == children.size() - 1);
        children[i]->printDFS(os, childPrefix, lastChild, false);
    }
}

string nodeTypeToString(NodeType type) {
    switch (type) {
        case PROGRAM: return "<program>";
        case PROGRAM_HEADER: return "<program-header>";
        case DECLARATION_PART: return "<declaration-part>";
        case CONST_DECLARATION: return "<const-declaration>";
        case CONSTANT: return "<constant>";
        case TYPE_DECLARATION: return "<type-declaration>";
        case VAR_DECLARATION: return "<var-declaration>";
        case IDENTIFIER_LIST: return "<identifier-list>";
        case TYPE: return "<type>";
        case ARRAY_TYPE: return "<array-type>";
        case RANGE: return "<range>";
        case ENUMERATED: return "<enumerated>";
        case RECORD_TYPE: return "<record-type>";
        case FIELD_LIST: return "<field-list>";
        case FIELD_PART: return "<field-part>";
        case SUBPROGRAM_DECLARATION: return "<subprogram-declaration>";
        case PROCEDURE_DECLARATION: return "<procedure-declaration>";
        case FUNCTION_DECLARATION: return "<function-declaration>";
        case BLOCK: return "<block>";
        case FORMAL_PARAMETER_LIST: return "<formal-parameter-list>";
        case PARAMETER_GROUP: return "<parameter-group>";
        case COMPOUND_STATEMENT: return "<compound-statement>";
        case STATEMENT_LIST: return "<statement-list>";
        case STATEMENT: return "<statement>";
        case VARIABLE: return "<variable>";
        case COMPONENT_VARIABLE: return "<component-variable>";
        case INDEX_LIST: return "<index-list>";
        case ASSIGNMENT_STATEMENT: return "<assignment-statement>";
        case IF_STATEMENT: return "<if-statement>";
        case CASE_STATEMENT: return "<case-statement>";
        case CASE_BLOCK: return "<case-block>";
        case WHILE_STATEMENT: return "<while-statement>";
        case REPEAT_STATEMENT: return "<repeat-statement>";
        case FOR_STATEMENT: return "<for-statement>";
        case PROCEDURE_FUNCTION_CALL: return "<procedure-function-call>";
        case PARAMETER_LIST: return "<parameter-list>";
        case EXPRESSION: return "<expression>";
        case SIMPLE_EXPRESSION: return "<simple-expression>";
        case TERM: return "<term>";
        case FACTOR: return "<factor>";
        case RELATIONAL_OPERATOR: return "<relational-operator>";
        case ADDITIVE_OPERATOR: return "<additive-operator>";
        case MULTIPLICATIVE_OPERATOR: return "<multiplicative-operator>";
        default: return "Unknown Node Type";
    }
}
