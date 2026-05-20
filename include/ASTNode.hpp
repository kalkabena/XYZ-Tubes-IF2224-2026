#pragma once
#include <bits/stdc++.h>
#include "Node.hpp"

using namespace std;


class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(std::ostream& os, std::string prefix, bool isLast) const = 0;
};

class BlockNode : public ASTNode {
public:
    vector<unique_ptr<ASTNode>> statements;
    
    void print(std::ostream& os, std::string prefix, bool isLast) const override {
        os << prefix << (isLast ? "└── " : "├── ") << "BlockNode\n";
        std::string childPrefix = prefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < statements.size(); ++i) {
            statements[i]->print(os, childPrefix, i == statements.size() - 1);
        }
    }
};

class NumberNode : public ASTNode {
public:
    string value;
    NumberNode(string val) : value(val) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override {
        os << prefix << (isLast ? "└── " : "├── ") << "NumberNode(" << value << ")\n";
    }
};

class StringNode : public ASTNode {
public:
    string value;
    StringNode(string val) : value(val) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override {
        os << prefix << (isLast ? "└── " : "├── ") << "StringNode(" << value << ")\n";
    }
};

class VariableNode : public ASTNode {
public:
    string name;
    VariableNode(string n) : name(n) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override {
        os << prefix << (isLast ? "└── " : "├── ") << "VariableNode(" << name << ")\n";
    }
};
class CallNode : public ASTNode {
public:
    string functionName;
    vector<unique_ptr<ASTNode>> arguments;

    CallNode(string name, vector<unique_ptr<ASTNode>> args) 
        : functionName(name), arguments(move(args)) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};

class AssignNode : public ASTNode {
public:
    string targetVariable;
    unique_ptr<ASTNode> value; 

    AssignNode(string target, unique_ptr<ASTNode> val) 
        : targetVariable(target), value(move(val)) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};
class BinOpNode : public ASTNode {
public:
    string op; 
    unique_ptr<ASTNode> left;
    unique_ptr<ASTNode> right;

    BinOpNode(string operation, unique_ptr<ASTNode> l, unique_ptr<ASTNode> r)
        : op(operation), left(move(l)), right(move(r)) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};

class IfNode : public ASTNode {
public:
    unique_ptr<ASTNode> condition;
    unique_ptr<ASTNode> thenBranch;
    unique_ptr<ASTNode> elseBranch; // Bisa nullptr

    IfNode(unique_ptr<ASTNode> cond, unique_ptr<ASTNode> thenB, unique_ptr<ASTNode> elseB = nullptr)
        : condition(move(cond)), thenBranch(move(thenB)), elseBranch(move(elseB)) {}

    void print(ostream& os, string prefix, bool isLast) const override;
};

class WhileNode : public ASTNode {
public:
    unique_ptr<ASTNode> condition;
    unique_ptr<ASTNode> body;
    WhileNode(unique_ptr<ASTNode> cond, unique_ptr<ASTNode> b) : condition(move(cond)), body(move(b)) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};

class RepeatNode : public ASTNode {
public:
    unique_ptr<ASTNode> body;
    unique_ptr<ASTNode> condition;
    RepeatNode(unique_ptr<ASTNode> b, unique_ptr<ASTNode> cond) : body(move(b)), condition(move(cond)) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};

class ForNode : public ASTNode {
public:
    string varName;
    unique_ptr<ASTNode> start;
    unique_ptr<ASTNode> end;
    unique_ptr<ASTNode> body;
    ForNode(string var, unique_ptr<ASTNode> s, unique_ptr<ASTNode> e, unique_ptr<ASTNode> b)
        : varName(var), start(move(s)), end(move(e)), body(move(b)) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};

class CaseNode : public ASTNode {
public:
    unique_ptr<ASTNode> expression;
    vector<pair<string, unique_ptr<ASTNode>>> branches;
    CaseNode(unique_ptr<ASTNode> expr) : expression(move(expr)) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};

class VarDeclNode : public ASTNode {
public:
    vector<string> varNames;
    string type;
    void print(ostream& os, string prefix, bool isLast) const override;
};

class SubprogramDeclNode : public ASTNode {
public:
    string name;
    unique_ptr<ASTNode> block;
    SubprogramDeclNode(string n, unique_ptr<ASTNode> b) : name(n), block(move(b)) {}
    void print(ostream& os, string prefix, bool isLast) const override;
};

class FieldAccessNode : public ASTNode {
public:
    string recordName;
    string fieldName;
    FieldAccessNode(string rName, string fName) : recordName(rName), fieldName(fName) {}
    void print(std::ostream& os, std::string prefix, bool isLast) const override;
};

class ArrayAccessNode : public ASTNode {
public:
    string arrayName;
    unique_ptr<ASTNode> indexExpression;

    ArrayAccessNode(string name, unique_ptr<ASTNode> idxExpr) 
        : arrayName(name), indexExpression(move(idxExpr)) {}
        
    void print(ostream& os, string prefix, bool isLast) const override;
};