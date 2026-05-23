#include "parsingTree.hpp"
#include <fstream>
#include <stdexcept>
#include <sstream>

using namespace std;

ParsingTree::ParsingTree(Lexer& lexInstance) : lexer(lexInstance), root(nullptr) {
    currentToken = lexer.getNextToken();
    while (currentToken.type == comment) {
        currentToken = lexer.getNextToken();
    }
	nextToken = lexer.getNextToken();
	while (nextToken.type == comment) {
        nextToken = lexer.getNextToken();
    }
}

void ParsingTree::advance() {
	currentToken = nextToken;
    nextToken = lexer.getNextToken();
	while (nextToken.type == comment) {
        nextToken = lexer.getNextToken();
    }
}

string ParsingTree::tokenLabel(const Token& token) const {
    string name = getTokenName(token.type);
    if (token.type == ident || token.type == intcon || token.type == realcon ||
        token.type == charcon || token.type == string_tok || token.type == unknown_tok) {
        return name + "(" + token.lexeme + ")";
    }
    return name;
}

unique_ptr<Node> ParsingTree::accept(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        auto leaf = make_unique<Node>(TOKEN_NODE, tokenLabel(currentToken));
        advance();
        return leaf;
    }
    return nullptr;
}

unique_ptr<Node> ParsingTree::expect(TokenType expectedType) {
    auto leaf = accept(expectedType);
    if (leaf) return leaf;

    throw runtime_error(
        "Syntax Error: expected " + getTokenName(expectedType) +
        ", but got " + getTokenName(currentToken.type) +
        " ('" + currentToken.lexeme + "')"
    );
}

bool ParsingTree::isStatementStart(TokenType type) const {
    return type == ident || type == ifsy || type == casesy || type == whilesy ||
           type == repeatsy || type == forsy || type == beginsy;
}

bool ParsingTree::isFactorStart(TokenType type) const {
    return type == ident || type == intcon || type == realcon || type == charcon ||
           type == string_tok || type == lparent || type == notsy;
}

bool ParsingTree::isRelationalOperator(TokenType type) const {
    return type == eql || type == neq || type == gtr || type == geq || type == lss || type == leq;
}

bool ParsingTree::isAdditiveOperator(TokenType type) const {
    return type == plus_tok || type == minus_tok || type == orsy;
}

bool ParsingTree::isMultiplicativeOperator(TokenType type) const {
    return type == times || type == rdiv || type == idiv || type == imod || type == andsy;
}

unique_ptr<Node> ParsingTree::parseProgram() {
    auto node = make_unique<Node>(PROGRAM);
    node->addChild(parseProgramHeader());
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(expect(period));
    return node;
}

unique_ptr<Node> ParsingTree::parseProgramHeader() {
    auto node = make_unique<Node>(PROGRAM_HEADER);
    node->addChild(expect(programsy));
    node->addChild(expect(ident));
    node->addChild(expect(semicolon));
    return node;
}

unique_ptr<Node> ParsingTree::parseDeclarationPart() {
    auto node = make_unique<Node>(DECLARATION_PART);
    while (currentToken.type == constsy) node->addChild(parseConstDeclaration());
    while (currentToken.type == typesy) node->addChild(parseTypeDeclaration());
    while (currentToken.type == varsy) node->addChild(parseVarDeclaration());
    while (currentToken.type == proceduresy || currentToken.type == functionsy) {
        node->addChild(parseSubprogramDeclaration());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseConstDeclaration() {
    auto node = make_unique<Node>(CONST_DECLARATION);
    node->addChild(expect(constsy));
    do {
        node->addChild(expect(ident));
        node->addChild(expect(eql));
        node->addChild(parseConstant());
        node->addChild(expect(semicolon));
    } while (currentToken.type == ident);
    return node;
}

unique_ptr<Node> ParsingTree::parseConstant() {
    auto node = make_unique<Node>(CONSTANT);

    if (currentToken.type == charcon) node->addChild(expect(charcon));
    else if (currentToken.type == string_tok) node->addChild(expect(string_tok));
    else {
        if (currentToken.type == plus_tok) node->addChild(expect(plus_tok));
        else if (currentToken.type == minus_tok) node->addChild(expect(minus_tok));

        if (currentToken.type == ident) node->addChild(expect(ident));
        else if (currentToken.type == intcon) node->addChild(expect(intcon));
        else if (currentToken.type == realcon) node->addChild(expect(realcon));
        else throw runtime_error("Syntax Error: expected constant");
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseTypeDeclaration() {
    auto node = make_unique<Node>(TYPE_DECLARATION);
    node->addChild(expect(typesy));
    do {
        node->addChild(expect(ident));
        node->addChild(expect(eql));
        node->addChild(parseType());
        node->addChild(expect(semicolon));
    } while (currentToken.type == ident);
    return node;
}

unique_ptr<Node> ParsingTree::parseVarDeclaration() {
    auto node = make_unique<Node>(VAR_DECLARATION);
    node->addChild(expect(varsy));
    do {
        node->addChild(parseIdentifierList());
        node->addChild(expect(colon));
        node->addChild(parseType());
        node->addChild(expect(semicolon));
    } while (currentToken.type == ident);
    return node;
}

unique_ptr<Node> ParsingTree::parseIdentifierList() {
    auto node = make_unique<Node>(IDENTIFIER_LIST);
    node->addChild(expect(ident));
    while (currentToken.type == comma) {
        node->addChild(expect(comma));
        node->addChild(expect(ident));
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseType() {
    auto node = make_unique<Node>(TYPE);
    if (currentToken.type == ident) node->addChild(expect(ident));
    else if (currentToken.type == arraysy) node->addChild(parseArrayType());
    else if (currentToken.type == lparent) node->addChild(parseEnumerated());
    else if (currentToken.type == recordsy) node->addChild(parseRecordType());
    else if (currentToken.type == intcon || currentToken.type == charcon ||
             currentToken.type == string_tok || currentToken.type == plus_tok ||
             currentToken.type == minus_tok) {
        node->addChild(parseRange());
    } else {
        throw runtime_error("Syntax Error: expected type");
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseArrayType() {
    auto node = make_unique<Node>(ARRAY_TYPE);
    node->addChild(expect(arraysy));
    node->addChild(expect(lbrack));
    if (currentToken.type == ident) node->addChild(expect(ident));
    else node->addChild(parseRange());
    node->addChild(expect(rbrack));
    node->addChild(expect(ofsy));
    node->addChild(parseType());
    return node;
}

unique_ptr<Node> ParsingTree::parseRange() {
    auto node = make_unique<Node>(RANGE);
    node->addChild(parseConstant());
    node->addChild(expect(period));
    node->addChild(expect(period));
    node->addChild(parseConstant());
    return node;
}

unique_ptr<Node> ParsingTree::parseEnumerated() {
    auto node = make_unique<Node>(ENUMERATED);
    node->addChild(expect(lparent));
    node->addChild(expect(ident));
    while (currentToken.type == comma) {
        node->addChild(expect(comma));
        node->addChild(expect(ident));
    }
    node->addChild(expect(rparent));
    return node;
}

unique_ptr<Node> ParsingTree::parseRecordType() {
    auto node = make_unique<Node>(RECORD_TYPE);
    node->addChild(expect(recordsy));
    node->addChild(parseFieldList());
    node->addChild(expect(endsy));
    return node;
}

unique_ptr<Node> ParsingTree::parseFieldList() {
    auto node = make_unique<Node>(FIELD_LIST);
    node->addChild(parseFieldPart());
    while (currentToken.type == semicolon) {
        node->addChild(expect(semicolon));
        if (currentToken.type == ident) node->addChild(parseFieldPart());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseFieldPart() {
    auto node = make_unique<Node>(FIELD_PART);
    node->addChild(parseIdentifierList());
    node->addChild(expect(colon));
    node->addChild(parseType());
    return node;
}

unique_ptr<Node> ParsingTree::parseSubprogramDeclaration() {
    auto node = make_unique<Node>(SUBPROGRAM_DECLARATION);
    if (currentToken.type == proceduresy) node->addChild(parseProcedureDeclaration());
    else if (currentToken.type == functionsy) node->addChild(parseFunctionDeclaration());
    else throw runtime_error("Syntax Error: expected procedure or function declaration");
    return node;
}

unique_ptr<Node> ParsingTree::parseProcedureDeclaration() {
    auto node = make_unique<Node>(PROCEDURE_DECLARATION);
    node->addChild(expect(proceduresy));
    node->addChild(expect(ident));
    if (currentToken.type == lparent) node->addChild(parseFormalParameterList());
    node->addChild(expect(semicolon));
    node->addChild(parseBlock());
    node->addChild(expect(semicolon));
    return node;
}

unique_ptr<Node> ParsingTree::parseFunctionDeclaration() {
    auto node = make_unique<Node>(FUNCTION_DECLARATION);
    node->addChild(expect(functionsy));
    node->addChild(expect(ident));
    if (currentToken.type == lparent) node->addChild(parseFormalParameterList());
    node->addChild(expect(colon));
    node->addChild(expect(ident));
    node->addChild(expect(semicolon));
    node->addChild(parseBlock());
    node->addChild(expect(semicolon));
    return node;
}

unique_ptr<Node> ParsingTree::parseBlock() {
    auto node = make_unique<Node>(BLOCK);
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    return node;
}

unique_ptr<Node> ParsingTree::parseFormalParameterList() {
    auto node = make_unique<Node>(FORMAL_PARAMETER_LIST);
    node->addChild(expect(lparent));
    node->addChild(parseParameterGroup());
    while (currentToken.type == semicolon) {
        node->addChild(expect(semicolon));
        node->addChild(parseParameterGroup());
    }
    node->addChild(expect(rparent));
    return node;
}

unique_ptr<Node> ParsingTree::parseParameterGroup() {
    auto node = make_unique<Node>(PARAMETER_GROUP);
    node->addChild(parseIdentifierList());
    node->addChild(expect(colon));
    if (currentToken.type == arraysy) node->addChild(parseArrayType());
    else node->addChild(expect(ident));
    return node;
}

unique_ptr<Node> ParsingTree::parseCompoundStatement() {
    auto node = make_unique<Node>(COMPOUND_STATEMENT);
    node->addChild(expect(beginsy));
    node->addChild(parseStatementList());
    node->addChild(expect(endsy));
    return node;
}

unique_ptr<Node> ParsingTree::parseStatementList() {
    auto node = make_unique<Node>(STATEMENT_LIST);
    if (isStatementStart(currentToken.type)) node->addChild(parseStatement());
    while (currentToken.type == semicolon) {
        node->addChild(expect(semicolon));
        if (isStatementStart(currentToken.type)) node->addChild(parseStatement());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseStatement() {
    auto node = make_unique<Node>(STATEMENT);
    if (currentToken.type == beginsy) node->addChild(parseCompoundStatement());
    else if (currentToken.type == ifsy) node->addChild(parseIfStatement());
    else if (currentToken.type == casesy) node->addChild(parseCaseStatement());
    else if (currentToken.type == whilesy) node->addChild(parseWhileStatement());
    else if (currentToken.type == repeatsy) node->addChild(parseRepeatStatement());
    else if (currentToken.type == forsy) node->addChild(parseForStatement());
    else if (currentToken.type == ident) {
        // ident bisa menjadi assignment: x := 5 / arr[1] := 5
        // atau procedure/function call: writeln(x)
        auto firstIdent = expect(ident);

        if (currentToken.type == lparent) {
            auto call = make_unique<Node>(PROCEDURE_FUNCTION_CALL);
            call->addChild(std::move(firstIdent));
            call->addChild(expect(lparent));
            if (currentToken.type != rparent) call->addChild(parseParameterList());
            call->addChild(expect(rparent));
            node->addChild(std::move(call));
        } else {
            auto varNode = make_unique<Node>(VARIABLE);
            varNode->addChild(std::move(firstIdent));
            while (currentToken.type == lbrack || currentToken.type == period) {
                varNode->addChild(parseComponentVariable());
            }

            auto assign = make_unique<Node>(ASSIGNMENT_STATEMENT);
            assign->addChild(std::move(varNode));
            assign->addChild(expect(becomes));
            assign->addChild(parseExpression());
            node->addChild(std::move(assign));
        }
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseVariable() {
    auto node = make_unique<Node>(VARIABLE);
    node->addChild(expect(ident));
    while (currentToken.type == lbrack || currentToken.type == period) {
        node->addChild(parseComponentVariable());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseComponentVariable() {
    auto node = make_unique<Node>(COMPONENT_VARIABLE);
    if (currentToken.type == lbrack) {
        node->addChild(expect(lbrack));
        node->addChild(parseIndexList());
        node->addChild(expect(rbrack));
    } else if (currentToken.type == period) {
        node->addChild(expect(period));
        node->addChild(expect(ident));
    } else {
        throw runtime_error("Syntax Error: expected component variable");
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseIndexList() {
    auto node = make_unique<Node>(INDEX_LIST);
    if (currentToken.type == intcon) node->addChild(expect(intcon));
    else if (currentToken.type == charcon) node->addChild(expect(charcon));
    else if (currentToken.type == ident) node->addChild(expect(ident));
    else throw runtime_error("Syntax Error: expected index");

    while (currentToken.type == comma) {
        node->addChild(expect(comma));
        if (currentToken.type == intcon) node->addChild(expect(intcon));
        else if (currentToken.type == charcon) node->addChild(expect(charcon));
        else if (currentToken.type == ident) node->addChild(expect(ident));
        else throw runtime_error("Syntax Error: expected index after comma");
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseAssignmentStatement() {
    auto node = make_unique<Node>(ASSIGNMENT_STATEMENT);
    node->addChild(parseVariable());
    node->addChild(expect(becomes));
    node->addChild(parseExpression());
    return node;
}

unique_ptr<Node> ParsingTree::parseIfStatement() {
    auto node = make_unique<Node>(IF_STATEMENT);
    node->addChild(expect(ifsy));
    node->addChild(parseExpression());
    node->addChild(expect(thensy));
    node->addChild(parseStatement());
    if (currentToken.type == elsesy) {
        node->addChild(expect(elsesy));
        node->addChild(parseStatement());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseCaseStatement() {
    auto node = make_unique<Node>(CASE_STATEMENT);
    node->addChild(expect(casesy));
    node->addChild(parseExpression());
    node->addChild(expect(ofsy));
    node->addChild(parseCaseBlock());
    node->addChild(expect(endsy));
    return node;
}

unique_ptr<Node> ParsingTree::parseCaseBlock() {
    auto node = make_unique<Node>(CASE_BLOCK);
    node->addChild(parseConstant());
    while (currentToken.type == comma) {
        node->addChild(expect(comma));
        node->addChild(parseConstant());
    }
    node->addChild(expect(colon));
    node->addChild(parseStatement());
    while (currentToken.type == semicolon) {
        node->addChild(expect(semicolon));
        if (currentToken.type == endsy) break;
        node->addChild(parseCaseBlock());
        break;
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseWhileStatement() {
    auto node = make_unique<Node>(WHILE_STATEMENT);
    node->addChild(expect(whilesy));
    node->addChild(parseExpression());
    node->addChild(expect(dosy));
    node->addChild(parseCompoundStatement());
    // node->addChild(expect(semicolon));
    return node;
}

unique_ptr<Node> ParsingTree::parseRepeatStatement() {
    auto node = make_unique<Node>(REPEAT_STATEMENT);
    node->addChild(expect(repeatsy));
    node->addChild(parseStatementList());
    node->addChild(expect(untilsy));
    node->addChild(parseExpression());
    return node;
}

unique_ptr<Node> ParsingTree::parseForStatement() {
    auto node = make_unique<Node>(FOR_STATEMENT);
    node->addChild(expect(forsy));
    node->addChild(expect(ident));
    node->addChild(expect(becomes));
    node->addChild(parseExpression());
    if (currentToken.type == tosy) node->addChild(expect(tosy));
    else node->addChild(expect(downtosy));
    node->addChild(parseExpression());
    node->addChild(expect(dosy));
    node->addChild(parseCompoundStatement());
    // node->addChild(expect(semicolon));
    return node;
}

unique_ptr<Node> ParsingTree::parseProcedureFunctionCall() {
    auto node = make_unique<Node>(PROCEDURE_FUNCTION_CALL);
    node->addChild(expect(ident));
    if (currentToken.type == lparent) {
        node->addChild(expect(lparent));
        if (currentToken.type != rparent) node->addChild(parseParameterList());
        node->addChild(expect(rparent));
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseParameterList() {
    auto node = make_unique<Node>(PARAMETER_LIST);
    node->addChild(parseExpression());
    while (currentToken.type == comma) {
        node->addChild(expect(comma));
        node->addChild(parseExpression());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseExpression() {
    auto node = make_unique<Node>(EXPRESSION);
    node->addChild(parseSimpleExpression());
    if (isRelationalOperator(currentToken.type)) {
        node->addChild(parseRelationalOperator());
        node->addChild(parseSimpleExpression());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseSimpleExpression() {
    auto node = make_unique<Node>(SIMPLE_EXPRESSION);
    if (currentToken.type == plus_tok) node->addChild(expect(plus_tok));
    else if (currentToken.type == minus_tok) node->addChild(expect(minus_tok));
    node->addChild(parseTerm());
    while (isAdditiveOperator(currentToken.type)) {
        node->addChild(parseAdditiveOperator());
        node->addChild(parseTerm());
    }
    return node;
}

unique_ptr<Node> ParsingTree::parseTerm() {
    auto node = make_unique<Node>(TERM);
    node->addChild(parseFactor());
    while (isMultiplicativeOperator(currentToken.type)) {
        node->addChild(parseMultiplicativeOperator());
        node->addChild(parseFactor());
    }
    return node;
}

    unique_ptr<Node> ParsingTree::parseFactor() {
        auto node = make_unique<Node>(FACTOR);
        if (currentToken.type == intcon) node->addChild(expect(intcon));
        else if (currentToken.type == realcon) node->addChild(expect(realcon));
        else if (currentToken.type == charcon) node->addChild(expect(charcon));
        else if (currentToken.type == string_tok) node->addChild(expect(string_tok));
        else if (currentToken.type == lparent) {
            node->addChild(expect(lparent));
            node->addChild(parseExpression());
            node->addChild(expect(rparent));
        } else if (currentToken.type == notsy) {
            node->addChild(expect(notsy));
            node->addChild(parseFactor());
        } else if (currentToken.type == ident) {
            // ident bisa function call atau variable. Keduanya dimakan dari ident.
            if (nextToken.type == lparent) node->addChild(parseProcedureFunctionCall());
            else node->addChild(parseVariable());
        } else {
            throw runtime_error("Syntax Error: expected factor, got " + getTokenName(currentToken.type));
        }
        return node;
    }

unique_ptr<Node> ParsingTree::parseRelationalOperator() {
    auto node = make_unique<Node>(RELATIONAL_OPERATOR);
    if (currentToken.type == eql) node->addChild(expect(eql));
    else if (currentToken.type == neq) node->addChild(expect(neq));
    else if (currentToken.type == gtr) node->addChild(expect(gtr));
    else if (currentToken.type == geq) node->addChild(expect(geq));
    else if (currentToken.type == lss) node->addChild(expect(lss));
    else if (currentToken.type == leq) node->addChild(expect(leq));
    else throw runtime_error("Syntax Error: expected relational operator");
    return node;
}

unique_ptr<Node> ParsingTree::parseAdditiveOperator() {
    auto node = make_unique<Node>(ADDITIVE_OPERATOR);
    if (currentToken.type == plus_tok) node->addChild(expect(plus_tok));
    else if (currentToken.type == minus_tok) node->addChild(expect(minus_tok));
    else if (currentToken.type == orsy) node->addChild(expect(orsy));
    else throw runtime_error("Syntax Error: expected additive operator");
    return node;
}

unique_ptr<Node> ParsingTree::parseMultiplicativeOperator() {
    auto node = make_unique<Node>(MULTIPLICATIVE_OPERATOR);
    if (currentToken.type == times) node->addChild(expect(times));
    else if (currentToken.type == rdiv) node->addChild(expect(rdiv));
    else if (currentToken.type == idiv) node->addChild(expect(idiv));
    else if (currentToken.type == imod) node->addChild(expect(imod));
    else if (currentToken.type == andsy) node->addChild(expect(andsy));
    else throw runtime_error("Syntax Error: expected multiplicative operator");
    return node;
}

void ParsingTree::build() {
    root = parseProgram();
}

void ParsingTree::printToCLI() {
    if (root) root->printDFS(cout);
}

void ParsingTree::exportToFile(const string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open file " << filename << "\n";
        return;
    }
    if (root) root->printDFS(outFile);
}
