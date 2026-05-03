
#pragma once
#include <bits/stdc++.h>

using namespace std;

enum NodeType {
    TOKEN_NODE,
    PROGRAM,
    PROGRAM_HEADER,
    DECLARATION_PART,
    CONST_DECLARATION,
    CONSTANT,
    TYPE_DECLARATION,
    VAR_DECLARATION,
    IDENTIFIER_LIST,
    TYPE,
    ARRAY_TYPE,
    RANGE,
    ENUMERATED,
    RECORD_TYPE,
    FIELD_LIST,
    FIELD_PART,
    SUBPROGRAM_DECLARATION,
    PROCEDURE_DECLARATION,
    FUNCTION_DECLARATION,
    BLOCK,
    FORMAL_PARAMETER_LIST,
    PARAMETER_GROUP,
    COMPOUND_STATEMENT,
    STATEMENT_LIST,
    STATEMENT,
    VARIABLE,
    COMPONENT_VARIABLE,
    INDEX_LIST,
    ASSIGNMENT_STATEMENT,
    IF_STATEMENT,
    CASE_STATEMENT,
    CASE_BLOCK,
    WHILE_STATEMENT,
    REPEAT_STATEMENT,
    FOR_STATEMENT,
    PROCEDURE_FUNCTION_CALL,
    PARAMETER_LIST,
    EXPRESSION,
    SIMPLE_EXPRESSION,
    TERM,
    FACTOR,
    RELATIONAL_OPERATOR,
    ADDITIVE_OPERATOR,
    MULTIPLICATIVE_OPERATOR
};


class Node {
private:
public:
    NodeType type;
    string lexeme;
    vector<std::unique_ptr<Node>> children;
    Node(NodeType t) : type(t), lexeme("") {}
    Node(NodeType t, std::string l) : type(t), lexeme(l) {}
    ~Node() = default;
    void addChild(unique_ptr<Node> child);
    void printDFS(std::ostream& os, std::string prefix = "", bool isLast = true, bool isRoot = true) const;
};

string nodeTypeToString(NodeType type);