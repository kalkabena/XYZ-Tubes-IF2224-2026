#pragma once

#include "Node.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <memory>
#include <iostream>
#include <string>

class ParsingTree {
private:
    Lexer& lexer;
    Token currentToken;
    Token nextToken;
    std::unique_ptr<Node> root;

    void advance();
    bool isStatementStart(TokenType type) const;
    bool isFactorStart(TokenType type) const;
    bool isRelationalOperator(TokenType type) const;
    bool isAdditiveOperator(TokenType type) const;
    bool isMultiplicativeOperator(TokenType type) const;
    std::string tokenLabel(const Token& token) const;

    std::unique_ptr<Node> accept(TokenType expectedType);
    std::unique_ptr<Node> expect(TokenType expectedType);

    std::unique_ptr<Node> parseProgram();
    std::unique_ptr<Node> parseProgramHeader();
    std::unique_ptr<Node> parseDeclarationPart();

    std::unique_ptr<Node> parseConstDeclaration();
    std::unique_ptr<Node> parseConstant();

    std::unique_ptr<Node> parseTypeDeclaration();
    std::unique_ptr<Node> parseVarDeclaration();
    std::unique_ptr<Node> parseIdentifierList();
    std::unique_ptr<Node> parseType();
    std::unique_ptr<Node> parseArrayType();
    std::unique_ptr<Node> parseRange();
    std::unique_ptr<Node> parseEnumerated();
    std::unique_ptr<Node> parseRecordType();
    std::unique_ptr<Node> parseFieldList();
    std::unique_ptr<Node> parseFieldPart();

    std::unique_ptr<Node> parseSubprogramDeclaration();
    std::unique_ptr<Node> parseProcedureDeclaration();
    std::unique_ptr<Node> parseFunctionDeclaration();
    std::unique_ptr<Node> parseBlock();
    std::unique_ptr<Node> parseFormalParameterList();
    std::unique_ptr<Node> parseParameterGroup();

    std::unique_ptr<Node> parseCompoundStatement();
    std::unique_ptr<Node> parseStatementList();
    std::unique_ptr<Node> parseStatement();

    std::unique_ptr<Node> parseVariable();
    std::unique_ptr<Node> parseComponentVariable();
    std::unique_ptr<Node> parseIndexList();

    std::unique_ptr<Node> parseAssignmentStatement();
    std::unique_ptr<Node> parseIfStatement();
    std::unique_ptr<Node> parseCaseStatement();
    std::unique_ptr<Node> parseCaseBlock();
    std::unique_ptr<Node> parseWhileStatement();
    std::unique_ptr<Node> parseRepeatStatement();
    std::unique_ptr<Node> parseForStatement();
    std::unique_ptr<Node> parseProcedureFunctionCall();

    std::unique_ptr<Node> parseParameterList();
    std::unique_ptr<Node> parseExpression();
    std::unique_ptr<Node> parseSimpleExpression();
    std::unique_ptr<Node> parseTerm();
    std::unique_ptr<Node> parseFactor();
    std::unique_ptr<Node> parseRelationalOperator();
    std::unique_ptr<Node> parseAdditiveOperator();
    std::unique_ptr<Node> parseMultiplicativeOperator();

public:
    ParsingTree(Lexer& lexInstance);
    ~ParsingTree() = default;

    void build();
    void printToCLI();
    void exportToFile(const std::string& filename);
};
