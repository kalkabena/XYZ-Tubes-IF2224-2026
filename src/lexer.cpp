#include "lexer.hpp"

using namespace std;

Lexer::Lexer(const string& src) : source(src), pos(0), dfa(true) {
    keywordMap["not"] = notsy;
    keywordMap["div"] = idiv;
    keywordMap["mod"] = imod;
    keywordMap["and"] = andsy;
    keywordMap["or"] = orsy;
    keywordMap["const"] = constsy;
    keywordMap["type"] = typesy;
    keywordMap["var"] = varsy;
    keywordMap["function"] = functionsy;
    keywordMap["procedure"] = proceduresy;
    keywordMap["array"] = arraysy;
    keywordMap["record"] = recordsy;
    keywordMap["program"] = programsy;
    keywordMap["begin"] = beginsy;
    keywordMap["if"] = ifsy;
    keywordMap["case"] = casesy;
    keywordMap["repeat"] = repeatsy;
    keywordMap["while"] = whilesy;
    keywordMap["for"] = forsy;
    keywordMap["end"] = endsy;
    keywordMap["else"] = elsesy;
    keywordMap["until"] = untilsy;
    keywordMap["of"] = ofsy;
    keywordMap["do"] = dosy;
    keywordMap["to"] = tosy;
    keywordMap["downto"] = downtosy;
    keywordMap["then"] = thensy;
}

void Lexer::printDFA_Graph() {
    dfa.printGraph();
}

Token Lexer::getNextToken() {
    while (pos < source.length() && std::isspace(source[pos])) pos++;
    
    if (pos >= source.length()) return {eof_tok, "EOF"};

    dfa.reset(); 
    char current = source[pos];

    // Identifiers and Keywords
    if (isalpha(current)) {
        string buffer = "";
        
        dfa.transition(source[pos], STATE_IDENT_BUILD);
        buffer += source[pos++];
        
        while (pos < source.length() && isalnum(source[pos])) {
            dfa.transition(source[pos], STATE_IDENT_BUILD);
            buffer += source[pos++];
        }
        
        string lower_buffer = buffer;
        for (char& c : lower_buffer) {
            c = std::tolower(static_cast<unsigned char>(c));
        }
        
        auto it = keywordMap.find(lower_buffer);
        if (it != keywordMap.end()) {
            return {it->second, buffer}; 
        }
        
        return {ident, buffer}; 
    }

    // Integers and Reals
    if (isdigit(current)) {
        string buffer = "";
        bool is_real = false;
        
        dfa.transition(source[pos], STATE_INT_BUILD);
        buffer += source[pos++];
        
        while (pos < source.length() && (isdigit(source[pos]) || source[pos] == '.')) {
            if (source[pos] == '.') {
                if (is_real) break; 
                
                if (pos + 1 < source.length() && source[pos+1] == '.') {
                    break; 
                }
                is_real = true;
                dfa.transition(source[pos], STATE_UNKNOWN); 
            } else {
                dfa.transition(source[pos], STATE_INT_BUILD);
            }
            buffer += source[pos++];
        }
        
        if (is_real) return {realcon, buffer};
        return {intcon, buffer}; 
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
        case '[':
            dfa.transition('[', STATE_UNKNOWN);
            return {lbrack, "["};
        case ']':
            dfa.transition(']', STATE_UNKNOWN);
            return {rbrack, "]"};
        case '{': {
            dfa.transition('{', STATE_UNKNOWN); 
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
                dfa.transition('*', STATE_UNKNOWN); 
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
        case '\'': {
            string raw_str = "";
            dfa.transition('\'', STATE_STRING); 
            
            while (pos < source.length() && source[pos] != '\'') {
                dfa.transition(source[pos], 21);
                raw_str += source[pos++];
            }
            
            if (pos < source.length() && source[pos] == '\'') {
                dfa.transition('\'', STATE_STRING);
                pos++; 
            } 
            
            if (raw_str.length() == 1) {
                return {charcon, "'" + raw_str + "'"};
            }
            return {string_tok, "'" + raw_str + "'"};
        }
        case ',':
            dfa.transition(',', STATE_COMMA);
            return {comma, ","};
        default: 
            dfa.transition(current, STATE_UNKNOWN);
            return {unknown_tok, std::string(1, current)};
    }
}

string getTokenName(TokenType type) {
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