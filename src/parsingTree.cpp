#include "parsingTree.hpp"
#include <fstream>
#include <stdexcept>

using namespace std;

ParsingTree::ParsingTree(Lexer& lexInstance) : lexer(lexInstance), root(nullptr) {
    advance();
}
void ParsingTree::advance() {
    currentToken = lexer.getNextToken();
}

unique_ptr<Node> ParsingTree::accept(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        auto leaf = make_unique<Node>(TOKEN_NODE, currentToken.lexeme);
        advance();
        return leaf;
    }
    return nullptr;
}

unique_ptr<Node> ParsingTree::expect(TokenType expectedType) {
    auto leaf = accept(expectedType);
    if (leaf) {
        return leaf;
    }
    
    // Halt menggunakan exception jika token wajib tidak ditemukan
    throw runtime_error("Syntax Error: Expected token type " + 
                             to_string(expectedType) + 
                             " but got " + to_string(currentToken.type) + 
                             " ('" + currentToken.lexeme + "')");
}

unique_ptr<Node> ParsingTree::parseProgram() {
    //program → program-header + declaration-part + compound-statement + period
    auto node = make_unique<Node>(PROGRAM);
    
    node->addChild(parseProgramHeader());
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(expect(period)); // Token terakhir wajib period (.)
    
    return node;
}

unique_ptr<Node> ParsingTree::parseProgramHeader() {
    //programsy + ident + semicolon
    auto node = make_unique<Node>(PROGRAM_HEADER);
    
    node->addChild(expect(programsy));
    node->addChild(expect(ident));
    node->addChild(expect(semicolon));
    
    return node;
}


unique_ptr<Node> ParsingTree::parseDeclarationPart() {
    //(const-declaration)* + (type-declaration)* + (var-declaration)* + (subprogram-declaration)*
    auto node = make_unique<Node>(DECLARATION_PART);
    
    node->addChild(parseConstDeclaration());
    node->addChild(parseTypeDeclaration());
    node->addChild(parseVarDeclaration());
    node->addChild(parseSubprogramDeclaration());
    return node;
}

unique_ptr<Node> ParsingTree::parseConstDeclaration() {
    //constsy + (ident + eql + constant + semicolon)+
    auto node = make_unique<Node>(CONST_DECLARATION);
    
    node->addChild(expect(constsy));
    node->addChild(parseConstant());
    node->addChild(expect(semicolon));
    
    return node;
}

unique_ptr<Node> ParsingTree::parseConstant() {
    //charcon | string | [(plus | minus)? + (ident | intcon | realcon)]
    auto node = make_unique<Node>(CONSTANT);
    if (expect(charcon)){
        
        node->addChild(expect(charcon));
        
    }
    else if (expect(string_tok)){
        
        node->addChild(expect(string_tok));
        
    }
    else if (expect(plus_tok) || expect(minus_tok)){
        
        node->addChild(expect(plus_tok));
        
        if (expect(ident)){
            
            node->addChild(expect(ident));
            
        }
        else if (expect(intcon)){
            
            node->addChild(expect(intcon));
            
        }
        else if (expect(realcon)){
        
            node->addChild(expect(realcon));
            
        }
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseTypeDeclaration() {
    //typesy + (ident + eql + type + semicolon)+
    auto node = make_unique<Node>(TYPE_DECLARATION);
    node->addChild(expect(typesy));
    node->addChild(parseType());
    node->addChild(expect(semicolon));  
    return node;
}

unique_ptr<Node> ParsingTree::parseVarDeclaration() {
    //varsy + (identifier-list + colon + type + semicolon)+
    auto node = make_unique<Node>(VAR_DECLARATION);
    node->addChild(expect(varsy));
    node->addChild(parseIdentifierList());
    node->addChild(expect(colon));
    node->addChild(parseType());
    node->addChild(expect(semicolon));
    return node;

}

unique_ptr<Node> ParsingTree::parseIdentifierList() {
    //ident + (comma + ident)*
    auto node = make_unique<Node>(IDENTIFIER_LIST);
    node->addChild(expect(ident));
    node->addChild(parseIdentifierList());
    return node;
}


unique_ptr<Node> ParsingTree::parseType() {
    // ident |array-type |range |enumerated |record-type   
    auto node = make_unique<Node>(TYPE);
    if (expect(ident)){
        node->addChild(expect(ident));
    }
    else if (expect(arraysy)){
        node->addChild(parseArrayType());
    }
    node->addChild(parseRange());
    node->addChild(parseEnumerated());
    node->addChild(parseRecordType());
    return node;
}

unique_ptr<Node> ParsingTree::parseArrayType(){
    //arraysy + lbrack + (range | ident) + rbrack + ofsy + type
    auto node = make_unique<Node>(ARRAY_TYPE);
    node->addChild(expect(arraysy));
    node->addChild(expect(lbrack));
    node->addChild(parseRange());
    node->addChild(expect(rbrack));
    node->addChild(expect(ofsy));
    node->addChild(parseType());
    return node;
}

unique_ptr<Node> ParsingTree::parseRange(){
    //constant + period + period + constant
    auto node = make_unique<Node>(RANGE);
    node->addChild(parseConstant());
    node->addChild(expect(period));
    node->addChild(expect(period));
    node->addChild(parseConstant());
    return node;
}


void ParsingTree::build() {
    root = parseProgram();
}

void ParsingTree::printToCLI() {
    if (root) {
        root->printDFS(cout);
    }
}

void ParsingTree::exportToFile(const string& filename) {
    ofstream outFile(filename);
    if (outFile.is_open() && root) {
        root->printDFS(outFile);
        outFile.close();
    } else {
        cerr << "Error: Could not open file " << filename << "\n";
    }
}