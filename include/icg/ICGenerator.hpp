#pragma once


#include <bits/stdc++.h>
#include <syntax/ASTNode.hpp>
#include <semantic/SymbolTable.hpp>
#include "TACInstruction.hpp"




class ICGenerator : public ASTVisitor{
private:
    std::vector<TACInstruction> code;
    int currentLine;
    SymbolTable& symbolTable;

public:
    ICGenerator(SymbolTable& symTab) : currentLine(0),symbolTable(symTab) {}
    int emit(const std::string& op, int level, int arg);
    void backpatch(int lineIndex, int targetLine);
    const std::vector<TACInstruction>& getCode() const { return code; }

    void printCode() const;
    void generate(ASTNode* root);
    void visit(BlockNode* node) override;
    void visit(NumberNode* node) override;
    void visit(StringNode*  /*node*/) override;
    void visit(VariableNode* node) override;
    void visit(CallNode* node) override;
    void visit(AssignNode* node) override;
    void visit(BinOpNode* node) override;
    void visit(IfNode* node) override;
    void visit(WhileNode* node) override;
    void visit(RepeatNode* node) override;
    void visit(ForNode* node) override;
    void visit(CaseNode* node) override;
    void visit(VarDeclNode* /*node*/) override;
    void visit(SubprogramDeclNode* node) override;
    void visit(FieldAccessNode* node) override;
    void visit(ArrayAccessNode* node) override;
};