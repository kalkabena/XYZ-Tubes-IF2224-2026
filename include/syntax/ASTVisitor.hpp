#pragma once

class BlockNode;
class NumberNode;
class StringNode;
class VariableNode;
class CallNode;
class AssignNode;
class BinOpNode;
class UnaryOpNode;
class IfNode;
class WhileNode;
class RepeatNode;
class ForNode;
class CaseNode;
class VarDeclNode;
class SubprogramDeclNode;
class FieldAccessNode;
class ArrayAccessNode;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(BlockNode* node) = 0;
    virtual void visit(NumberNode* node) = 0;
    virtual void visit(StringNode* node) = 0;
    virtual void visit(VariableNode* node) = 0;
    virtual void visit(CallNode* node) = 0;
    virtual void visit(AssignNode* node) = 0;
    virtual void visit(BinOpNode* node) = 0;
    virtual void visit(UnaryOpNode* node) = 0;
    virtual void visit(IfNode* node) = 0;
    virtual void visit(WhileNode* node) = 0;
    virtual void visit(RepeatNode* node) = 0;
    virtual void visit(ForNode* node) = 0;
    virtual void visit(CaseNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(SubprogramDeclNode* node) = 0;
    virtual void visit(FieldAccessNode* node) = 0;
    virtual void visit(ArrayAccessNode* node) = 0;
};