#pragma once
#include "dfa_graph.hpp"

#include <bits/stdc++.h>


enum TokenType {
    // Identifiers
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
    std::string lexeme;
};