#pragma once
#include "ASTNode.hpp"
#include "Node.hpp"
#include <memory>
#include <string>

using namespace std;

class AST_Tree {
public:
    unique_ptr<ASTNode> build(Node* cstRoot);

private:
    unique_ptr<ASTNode> buildExpression(Node* cstNode);
    unique_ptr<ASTNode> buildAssignment(Node* cstNode);
    unique_ptr<ASTNode> buildFunctionCall(Node* cstNode);
    unique_ptr<ASTNode> buildIfStatement(Node* cstNode);
    unique_ptr<ASTNode> buildWhileStatement(Node* cstNode);
    unique_ptr<ASTNode> buildRepeatStatement(Node* cstNode);
    unique_ptr<ASTNode> buildForStatement(Node* cstNode);
    unique_ptr<ASTNode> buildCaseStatement(Node* cstNode);
    unique_ptr<ASTNode> buildVarDeclaration(Node* cstNode);
    unique_ptr<ASTNode> buildProcedureDeclaration(Node* cstNode);
    unique_ptr<ASTNode> buildFunctionDeclaration(Node* cstNode);
    unique_ptr<ASTNode> buildCompoundStatement(Node* cstNode);
    unique_ptr<ASTNode> buildEmptyStatement(Node* cstNode);
    unique_ptr<ASTNode> buildFieldAccess(Node* cstNode);
    unique_ptr<ASTNode> buildComponentVariable(Node* cstNode);
    string extractVarName(Node* cstNode);
};