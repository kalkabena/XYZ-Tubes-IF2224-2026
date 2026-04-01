#pragma once
#include "dfa_graph.hpp"
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <iostream>
using namespace std;
enum TokenType {
    // Literals & Identifiers
    intcon, realcon, charcon, string_tok, ident,
    
    // Operators
    notsy, plus_tok, minus_tok, times, idiv, rdiv, imod, andsy, orsy,
    eql, neq, gtr, geq, lss, leq, becomes,
    
    // Delimiters & Punctuation
    lparent, rparent, lbrack, rbrack, comma, semicolon, period, colon,
    
    // Keywords
    constsy, typesy, varsy, functionsy, proceduresy, arraysy, recordsy,
    programsy, beginsy, ifsy, casesy, repeatsy, whilesy, forsy, endsy,
    elsesy, untilsy, ofsy, dosy, tosy, downtosy, thensy,
    
    // Special
    comment, eof_tok, unknown_tok
};

struct Token {
    TokenType type;
    string lexeme;
};

class Lexer {
private:
    string source;
    size_t pos;
    unordered_map<string, TokenType> keywordMap;
    DFA dfa;

public:
    Lexer(const string& src) : source(src), pos(0),dfa(true) {
        keywordMap["program"] = programsy;
        keywordMap["var"]=varsy;
        keywordMap["begin"] = beginsy;
        keywordMap["end"] = endsy;
        keywordMap["if"] = ifsy;
        keywordMap["case"] = casesy;
        keywordMap ["while"] = whilesy;
        keywordMap ["until"] = untilsy;
        keywordMap ["then"] = thensy;
        keywordMap ["else"] = elsesy;
        keywordMap ["begin"] = beginsy;
        keywordMap ["for"] = forsy;
        keywordMap ["of"] = ofsy;
        keywordMap ["function"] = functionsy;
        keywordMap ["procedure"] = proceduresy;
        keywordMap ["array"] = arraysy;
        keywordMap ["type"] = typesy;
        keywordMap ["const"] = constsy;
        keywordMap ["begin"] = beginsy;
        keywordMap ["record"] = recordsy;
    }
    void printDFA_Graph(){
        dfa.printGraph();
    }
    Token getNextToken() {
        while (pos < source.length() && std::isspace(source[pos])) pos++;
        
        if (pos >= source.length()) return {eof_tok, "EOF"};

        dfa.reset(); // Reset DFA state to 0 for the new token
        char current = source[pos];

        // Identifiers and Keywords
        if (isalpha(current)) {
            string buffer = "";
            
            // Log transition for the first letter
            dfa.transition(source[pos], STATE_IDENT_BUILD);
            buffer += source[pos++];
            
            // Log transitions for subsequent alphanumeric characters
            while (pos < source.length() && isalnum(source[pos])) {
                dfa.transition(source[pos], STATE_IDENT_BUILD);
                buffer += source[pos++];
            }
            
            auto it = keywordMap.find(buffer);
            if (it != keywordMap.end()) {
                return {it->second, buffer}; // Keyword token
            }
            
            return {ident, buffer}; // Identifier token
        }

        if (isdigit(current)) {
            string buffer = "";
            
            // Log transition for the first digit
            dfa.transition(source[pos], STATE_INT_BUILD);
            buffer += source[pos++];
            
            // Log transitions for subsequent digits
            while (pos < source.length() && isdigit(source[pos])) {
                dfa.transition(source[pos], STATE_INT_BUILD);
                buffer += source[pos++];
            }
            
            return {intcon, buffer}; // Integer token
        }

        pos++;
        switch (current) {
            case ';': 
                dfa.transition(';', STATE_SEMICOLON);
                return {semicolon, ";"};
            case '.': 
                dfa.transition('.', STATE_PERIOD);
                return {period, "."}; 
            case ':':
                dfa.transition(':', STATE_COLON, "Goten: Colon");
                if (pos < source.length() && source[pos] == '=') {
                    char next = source[pos++];
                    dfa.transition(next, STATE_BECOMES, "Goten: Becomes");
                    return {becomes, ":="};
                }
                return {colon, ":"};
            case '+': 
                dfa.transition('+', STATE_PLUS);
                return {plus_tok, "+"};
            case '-': 
                dfa.transition('-', STATE_MINUS);
                return {minus_tok, "-"};
            case '*': 
                dfa.transition('*', STATE_TIMES);
                return {times, "*"};
            case '/': 
                dfa.transition('/', STATE_RDIV);
                return {rdiv, "/"};
            case '=': 
                dfa.transition('=', STATE_EQUAL);
                if (pos < source.length() && source[pos] == '=') { 
                    char next = source[pos++];
                    dfa.transition(next, STATE_DOUBLE_EQUAL);
                    return {eql, "=="}; 
                }
                // A single '=' is invalid in your current specification
                return {unknown_tok, "="};
            case '<':
                dfa.transition('<', STATE_LESS);
                if (pos < source.length() && source[pos] == '>') { 
                    dfa.transition('>', STATE_NEQ);
                    pos++; 
                    return {neq, "<>"}; 
                }
                if (pos < source.length() && source[pos] == '=') { 
                    dfa.transition('=', STATE_LEQ);
                    pos++; 
                    return {leq, "<="}; 
                }
                return {lss, "<"};
            case '>':
                dfa.transition('>', STATE_GREATER);
                if (pos < source.length() && source[pos] == '=') { 
                    dfa.transition('=', STATE_GEQ);
                    pos++; 
                    return {geq, ">="}; 
                }
                return {gtr, ">"};
            case '{': {
                dfa.transition('{', STATE_UNKNOWN); // Update with comment state if available
                string buffer = "{";
                while (pos < source.length() && source[pos] != '}') {
                    buffer += source[pos++];
                }
                if (pos < source.length()) {
                    buffer += source[pos++]; 
                }
                return {comment, buffer};
            }
            case '(': {
                dfa.transition('(', STATE_LPAREN); 

                if (pos < source.length() && source[pos] == '*') {
                    string buffer = "(*";
                    dfa.transition('*', STATE_UNKNOWN); // Update to a comment state if you create one
                    pos++;
                    while (pos < source.length()) {
                        if (source[pos] == '*' && (pos + 1 < source.length()) && source[pos + 1] == ')') {
                            dfa.transition('*', STATE_UNKNOWN);
                            dfa.transition(')', STATE_UNKNOWN);
                            buffer += "*)";
                            pos += 2; 
                            return {comment, buffer};
                        }
                        dfa.transition(source[pos], STATE_UNKNOWN);
                        buffer += source[pos++];
                    }
                    return {comment, buffer};
                }
                return {lparent, "("};
            }
            case ')': 
                dfa.transition(')', STATE_RPAREN);
                return {rparent, ")"};
            case '\'':{
                string buffer = "'";
                dfa.transition('\'', STATE_STRING); // Log opening quote
                
                // Process characters inside the string
                while (pos < source.length() && source[pos] != '\'') {
                    dfa.transition(source[pos], 21);
                    buffer += source[pos++];
                }
                
                // Process closing quote
                if (pos < source.length() && source[pos] == '\'') {
                    dfa.transition('\'', STATE_STRING);
                    buffer += source[pos++];
                } 
                
                return {string_tok, buffer};
            }
            case ',':
                dfa.transition(',',STATE_COMMA);
                return {comma,","};
            default: 
                dfa.transition(current, STATE_UNKNOWN);
                return {unknown_tok, std::string(1, current)};
        }
    }
    };

string getTokenName(TokenType type){
    switch (type) {
        case intcon: return "intcon";
        case realcon: return "realcon";
        case charcon: return "charcon";
        case string_tok: return "string";
        case ident: return "ident";
        case notsy: return "notsy";
        case plus_tok: return "plus";
        case minus_tok: return "minus";
        case times: return "times";
        case idiv: return "idiv";
        case rdiv: return "rdiv";
        case imod: return "imod";
        case andsy: return "andsy";
        case orsy: return "orsy";
        case eql: return "eql";
        case neq: return "neq";
        case gtr: return "gtr";
        case geq: return "geq";
        case lss: return "lss";
        case leq: return "leq";
        case becomes: return "becomes";
        case lparent: return "lparent";
        case rparent: return "rparent";
        case lbrack: return "lbrack";
        case rbrack: return "rbrack";
        case comma: return "comma";
        case semicolon: return "semicolon";
        case period: return "period";
        case colon: return "colon";
        case constsy: return "constsy";
        case typesy: return "typesy";
        case varsy: return "varsy";
        case functionsy: return "functionsy";
        case proceduresy: return "proceduresy";
        case arraysy: return "arraysy";
        case recordsy: return "recordsy";
        case programsy: return "programsy";
        case beginsy: return "beginsy";
        case ifsy: return "ifsy";
        case casesy: return "casesy";
        case repeatsy: return "repeatsy";
        case whilesy: return "whilesy";
        case forsy: return "forsy";
        case endsy: return "endsy";
        case elsesy: return "elsesy";
        case untilsy: return "untilsy";
        case ofsy: return "ofsy";
        case dosy: return "dosy";
        case tosy: return "tosy";
        case downtosy: return "downtosy";
        case thensy: return "thensy";
        case comment: return "comment";
        case eof_tok: return "EOF";
        default: return "unknown_tok";
    }
}