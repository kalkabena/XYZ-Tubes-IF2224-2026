#include "AST_Tree.hpp"
#include <stdexcept>

// Definisi kelas pembantu lokal (Concrete Terminal Nodes) yang belum ada di ASTNode.hpp
class NumberNode : public ASTNode {
public:
    string value;
    NumberNode(string val) : value(val) {}
};

class StringNode : public ASTNode {
public:
    string value;
    StringNode(string val) : value(val) {}
};

class VariableNode : public ASTNode {
public:
    string name;
    VariableNode(string n) : name(n) {}
};

unique_ptr<ASTNode> AST_Tree::build(Node* cstRoot) {
    if (!cstRoot) return nullptr;

    if (cstRoot->type == COMPOUND_STATEMENT || cstRoot->type == STATEMENT_LIST || cstRoot->type == PROGRAM) {
        for (auto& child : cstRoot->children) {
            auto result = build(child.get());
            if (result) return result;
        }
    }

    if (cstRoot->type == STATEMENT) {
        if (!cstRoot->children.empty()) {
            return build(cstRoot->children[0].get());
        }
    }

    if (cstRoot->type == ASSIGNMENT_STATEMENT) {
        return buildAssignment(cstRoot);
    }

    if (cstRoot->type == PROCEDURE_FUNCTION_CALL) {
        return buildFunctionCall(cstRoot);
    }

    return nullptr;
}

unique_ptr<ASTNode> AST_Tree::buildAssignment(Node* cstNode) {
    string varName = extractVarName(cstNode->children[0].get());
    
    Node* exprNode = nullptr;
    for (auto& child : cstNode->children) {
        if (child->type == EXPRESSION) {
            exprNode = child.get();
            break;
        }
    }

    return make_unique<AssignNode>(varName, buildExpression(exprNode));
}

unique_ptr<ASTNode> AST_Tree::buildFunctionCall(Node* cstNode) {
    string funcName = cstNode->children[0]->lexeme;
    vector<unique_ptr<ASTNode>> args;

    for (auto& child : cstNode->children) {
        if (child->type == PARAMETER_LIST) {
            for (auto& pChild : child->children) {
                if (pChild->type == EXPRESSION) {
                    args.push_back(buildExpression(pChild.get()));
                }
            }
        }
    }

    return make_unique<CallNode>(funcName, move(args));
}

unique_ptr<ASTNode> AST_Tree::buildExpression(Node* cstNode) {
    if (!cstNode) return nullptr;

    if (cstNode->type == EXPRESSION) {
        if (cstNode->children.size() == 1) {
            return buildExpression(cstNode->children[0].get());
        }
        string op = cstNode->children[1]->lexeme;
        return make_unique<BinOpNode>(op, buildExpression(cstNode->children[0].get()), buildExpression(cstNode->children[2].get()));
    }

    if (cstNode->type == SIMPLE_EXPRESSION) {
        if (cstNode->children.size() == 1) {
            return buildExpression(cstNode->children[0].get());
        }
        string op = cstNode->children[1]->lexeme;
        return make_unique<BinOpNode>(op, buildExpression(cstNode->children[0].get()), buildExpression(cstNode->children[2].get()));
    }

    if (cstNode->type == TERM) {
        if (cstNode->children.size() == 1) {
            return buildExpression(cstNode->children[0].get());
        }
        string op = cstNode->children[1]->lexeme;
        return make_unique<BinOpNode>(op, buildExpression(cstNode->children[0].get()), buildExpression(cstNode->children[2].get()));
    }

    if (cstNode->type == FACTOR) {
        Node* primary = cstNode->children[0].get();
        
        if (primary->lexeme == "(") {
            return buildExpression(cstNode->children[1].get());
        }
        
        if (primary->type == TOKEN_NODE || primary->type == VARIABLE) {
            if (!primary->lexeme.empty() && primary->lexeme[0] == '\'') {
                return make_unique<StringNode>(primary->lexeme);
            }
            if (isdigit(primary->lexeme[0])) {
                return make_unique<NumberNode>(primary->lexeme);
            }
            return make_unique<VariableNode>(primary->lexeme);
        }
        return buildExpression(primary);
    }

    return nullptr;
}

string AST_Tree::extractVarName(Node* cstNode) {
    if (!cstNode) return "";
    if (cstNode->type == TOKEN_NODE || cstNode->type == VARIABLE) {
        return cstNode->lexeme;
    }
    if (!cstNode->children.empty()) {
        return extractVarName(cstNode->children[0].get());
    }
    return cstNode->lexeme;
}