#pragma once
#include "syntax/Node.hpp"
#include "semantic/SymbolTable.hpp"
#include <string>
#include <vector>

class SemanticAnalyzer {
private:
    SymbolTable& symbolTable;
    int currentLev = 0;
    int currentBlock = 0;

    std::vector<std::string> errors;
    void traverseNode(Node* node, int lev);
    DataType resolveTypeFromNode(Node* typeNode) const;
    std::string extractRawValue(const std::string& formattedLexeme) const;
    int buildArrayEntry(Node* arrayTypeNode);
    DataType getExprType(ASTNode* exprNode);
    void checkTypeCompatibility(ASTNode* astNode);

public:
    SemanticAnalyzer(SymbolTable& table);
    ~SemanticAnalyzer() = default;

    void reportError(const std::string& message);
    bool hasError() const;
    void printErrors() const;
    bool isDeclared(const std::string& name) const;
    const std::vector<std::string>& getErrors() const;
    void analyze(Node* cstRoot);
    void analyzeAST(ASTNode* astRoot);
};