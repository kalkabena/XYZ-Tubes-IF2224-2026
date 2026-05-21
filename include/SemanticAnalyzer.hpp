#pragma once
#include "Node.hpp"
#include "SymbolTable.hpp"
#include <string>

class SemanticAnalyzer {
private:
    SymbolTable& symbolTable;
    int currentLev = 0;
    int currentBlock = 0;

    // Fungsi internal untuk pemrosesan navigasi pohon dan parsing data semantik
    void traverseNode(Node* node, int lev);
    DataType resolveTypeFromNode(Node* typeNode) const;
    std::string extractRawValue(const std::string& formattedLexeme) const;
    int buildArrayEntry(Node* arrayTypeNode);

public:
    SemanticAnalyzer(SymbolTable& table);
    ~SemanticAnalyzer() = default;

    // Titik masuk utama untuk menganalisis pohon sintaksis konkrit (CST)
    void analyze(Node* cstRoot);
};