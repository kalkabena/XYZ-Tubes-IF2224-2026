#include "token.hpp"
#include <bits/stdc++.h>
int main() {
    ifstream inputFile("kode_sumber/kode_sumber.txt");
    if (!inputFile.is_open()) {
        cerr << "Error: Could not open the file 'input.txt'\n";
        return 1; 
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();
    string source_code = buffer.str();

    inputFile.close();
        
    Lexer lexer(source_code);
    Token t;

    do {
        t = lexer.getNextToken();

        if (t.type == eof_tok) break;
        if (t.type == ident || t.type == intcon|| t.type == string_tok) {
            cout << getTokenName(t.type) << " (" << t.lexeme << ")\n";
        } else {
            cout << getTokenName(t.type) << "\n";
        }

    } while (t.type != eof_tok);
    lexer.printDFA_Graph();

    return 0;
}