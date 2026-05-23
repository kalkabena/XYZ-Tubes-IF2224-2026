#pragma once
#include "token.hpp"
#include "dfa_graph.hpp"

#include <bits/stdc++.h>

class Lexer {
private:
    std::string source;
    size_t pos;
    std::unordered_map<std::string, TokenType> keywordMap;
    DFA dfa;
    Token peekBuffer;
    bool hasPeek = false;

    Token scanToken();
public:
    Lexer(const std::string& src);
    void printDFA_Graph(std::ostream& out);
    Token peekToken();
    Token getNextToken();
};

string getTokenName(TokenType type);