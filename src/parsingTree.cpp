#include "parsingTree.hpp"
#include <fstream>
#include <stdexcept>


ParsingTree::ParsingTree(Lexer& lexInstance) : lexer(lexInstance), root(nullptr) {
    advance();
}
void ParsingTree::advance() {
    currentToken = lexer.getNextToken();
}

std::unique_ptr<Node> ParsingTree::accept(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        auto leaf = std::make_unique<Node>(TOKEN_NODE, currentToken.lexeme);
        advance();
        return leaf;
    }
    return nullptr;
}

std::unique_ptr<Node> ParsingTree::expect(TokenType expectedType) {
    auto leaf = accept(expectedType);
    if (leaf) {
        return leaf;
    }
    
    // Halt menggunakan exception jika token wajib tidak ditemukan
    throw std::runtime_error("Syntax Error: Expected token type " + 
                             std::to_string(expectedType) + 
                             " but got " + std::to_string(currentToken.type) + 
                             " ('" + currentToken.lexeme + "')");
}

// --- Recursive Descent Implementations ---

std::unique_ptr<Node> ParsingTree::parseProgram() {
    auto node = std::make_unique<Node>(PROGRAM);
    
    node->addChild(parseProgramHeader());
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(expect(period)); // Token terakhir wajib period (.)
    
    return node;
}

std::unique_ptr<Node> ParsingTree::parseProgramHeader() {
    auto node = std::make_unique<Node>(PROGRAM_HEADER);
    
    node->addChild(expect(programsy));
    node->addChild(expect(ident));
    node->addChild(expect(semicolon));
    
    return node;
}







void ParsingTree::build() {
    root = parseProgram();
}

void ParsingTree::printToCLI() {
    if (root) {
        root->printDFS(std::cout);
    }
}

void ParsingTree::exportToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    if (outFile.is_open() && root) {
        root->printDFS(outFile);
        outFile.close();
    } else {
        std::cerr << "Error: Could not open file " << filename << "\n";
    }
}