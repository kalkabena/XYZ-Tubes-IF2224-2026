#pragma once
#include "ASTNode.hpp"
#include "Node.hpp"

class AST_Tree {
public:
    unique_ptr<ASTNode> build(Node* cstRoot);

private:
    unique_ptr<ASTNode> buildExpression(Node* cstNode);
    unique_ptr<ASTNode> buildAssignment(Node* cstNode);
    unique_ptr<ASTNode> buildFunctionCall(Node* cstNode);
    string extractVarName(Node* cstNode);
};