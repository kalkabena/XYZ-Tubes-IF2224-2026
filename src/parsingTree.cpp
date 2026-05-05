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

std::unique_ptr<Node> parseEnumerated(){
    //lparent + ident + (comma + ident)* + rparent
    auto node = std::make_unique<Node>(ENUMERATED);
    
    node->addChild(expect(lparent));
    node->addChild(expect(ident));
    
    while (currentToken.type == comma) {
        node->addChild(expect(comma));
        node->addChild(expect(ident));
    }
    
    node->addChild(expect(rparent));
    return node;
}

std::unique_ptr<Node> parseRecordType(){
    //recordsy + field-list + endsy
    auto node = std::make_unique<Node>(RECORD_TYPE);
    
    node->addChild(expect(recordsy));
    node->addChild(parseFieldList());
    node->addChild(expect(endsy));
    
    return node;
}

std::unique_ptr<Node> parseFieldList(){
    //field-part + (semicolon + field-part)*
    auto node = std::make_unique<Node>(FIELD_LIST);
    
    node->addChild(parseFieldPart());
    
    while (currentToken.type == semicolon) {
        node->addChild(expect(semicolon));
        if (currentToken.type == ident) {
            node->addChild(parseFieldPart());
        }
    }
    
    return node;
}

std::unique_ptr<Node> parseFieldPart(){
    //identifier-list + colon + type
    auto node = std::make_unique<Node>(FIELD_PART);
    
    node->addChild(parseIdentifierList());
    node->addChild(expect(colon));
    node->addChild(parseType());
    
    return node;
}

// Deklarasi Subprogram
std::unique_ptr<Node> parseSubprogramDeclaration(){
    //procedure-declaration | function-declaration
    auto node = std::make_unique<Node>(SUBPROGRAM_DECLARATION);
    
    if (currentToken.type == proceduresy) {
        node->addChild(parseProcedureDeclaration());
    } else if (currentToken.type == functionsy) {
        node->addChild(parseFunctionDeclaration());
    }
    
    return node;
}

std::unique_ptr<Node> parseProcedureDeclaration(){
    //proceduresy + ident + (formal-parameter-list)? + semicolon + block + semicolon
    auto node = std::make_unique<Node>(PROCEDURE_DECLARATION);
    
    node->addChild(expect(proceduresy));
    node->addChild(expect(ident));
    
    if (currentToken.type == lparent) {
        node->addChild(parseFormalParameterList());
    }
    
    node->addChild(expect(semicolon));
    node->addChild(parseBlock());
    node->addChild(expect(semicolon));
    
    return node;
}

std::unique_ptr<Node> parseFunctionDeclaration(){
    //functionsy + ident + (formal-parameter-list)? + colon + ident + semicolon+ block + semicolon
    auto node = std::make_unique<Node>(FUNCTION_DECLARATION);
    
    node->addChild(expect(functionsy));
    node->addChild(expect(ident));
    
    if (currentToken.type == lparent) {
        node->addChild(parseFormalParameterList());
    }
    
    node->addChild(expect(colon));
    node->addChild(expect(ident)); // Return type
    node->addChild(expect(semicolon));
    node->addChild(parseBlock());
    node->addChild(expect(semicolon));
    
    return node;
}

// Blok
std::unique_ptr<Node> parseBlock(){
    //declaration-part + compound-statement
    auto node = std::make_unique<Node>(BLOCK);
    
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    
    return node;
}

std::unique_ptr<Node> parseFormalParameterList(){
    //lparent + parameter-group + (semicolon + parameter-group)* + rparent
    auto node = std::make_unique<Node>(FORMAL_PARAMETER_LIST);
    
    node->addChild(expect(lparent));
    node->addChild(parseParameterGroup());
    
    while (currentToken.type == semicolon) {
        node->addChild(expect(semicolon));
        node->addChild(parseParameterGroup());
    }
    
    node->addChild(expect(rparent));
    return node;
}

std::unique_ptr<Node> parseParameterGroup(){
    //identifier-list + colon + (ident | array-type)
    auto node = std::make_unique<Node>(PARAMETER_GROUP);
    
    node->addChild(parseIdentifierList());
    node->addChild(expect(colon));
    
    if (currentToken.type == arraysy) {
        node->addChild(parseArrayType());
    } else {
        node->addChild(expect(ident));
    }
    
    return node;
}

// Deklarasi Blok
std::unique_ptr<Node> parseCompoundStatement();{
    //beginsy + statement-list + endsy
    auto node = std::make_unique<Node>(COMPOUND_STATEMENT);
    
    node->addChild(expect(beginsy));
    node->addChild(parseStatementList());
    node->addChild(expect(endsy));
    
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