#pragma once
#include <bits/stdc++.h>
#include "Node.hpp"

using namespace std;


class ASTNode {
public:
    virtual ~ASTNode() = default;

    //->virtual method seperti print() atau evaluate()
};


class CallNode : public ASTNode {
public:
    string functionName;
    vector<unique_ptr<ASTNode>> arguments;

    CallNode(string name, vector<unique_ptr<ASTNode>> args) 
        : functionName(name), arguments(move(args)) {}
};

class AssignNode : public ASTNode {
public:
    string targetVariable;
    unique_ptr<ASTNode> value; 

    AssignNode(string target, unique_ptr<ASTNode> val) 
        : targetVariable(target), value(move(val)) {}
};
class BinOpNode : public ASTNode {
public:
    string op; 
    unique_ptr<ASTNode> left;
    unique_ptr<ASTNode> right;

    BinOpNode(string operation, unique_ptr<ASTNode> l, unique_ptr<ASTNode> r)
        : op(operation), left(move(l)), right(move(r)) {}
};