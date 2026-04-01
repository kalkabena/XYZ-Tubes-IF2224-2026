#pragma once
#include <bits/stdc++.h>
using namespace std;

enum DfaState {
    STATE_START = 0,
    
    // Alphanumeric & Numbers
    STATE_IDENT_BUILD = 1,
    STATE_INT_BUILD = 3,
    STATE_REAL_DOT = 4,
    STATE_REAL_BUILD = 5,
    // Multi-character Operators 
    STATE_COLON = 10,       // ':'
    STATE_BECOMES = 11,     // ':='
    STATE_LESS = 12,        // '<'
    STATE_LEQ = 13,         // '<='
    STATE_NEQ = 14,         // '<>'
    STATE_GREATER = 15,     // '>'
    STATE_GEQ = 16,         // '>='
    STATE_EQUAL = 17,       // '='
    STATE_DOUBLE_EQUAL = 18, // '=='
    STATE_SEMICOLON = 20,
    STATE_STRING = 21,
    STATE_COMMA = 22,
    STATE_LPAREN = 26,
    STATE_RPAREN = 27,
    // Mathematical Operators 
    STATE_PLUS = 30,        // '+'
    STATE_MINUS = 31,       // '-'
    STATE_TIMES = 32,       // '*'
    STATE_RDIV = 33,        // '/'
    // Single-character States
    
    STATE_PERIOD = 40,
    
    STATE_UNKNOWN = 99
};
class DFA {
private:
    int currentState;
    bool debug;
    vector<string> transitionHistory; // Stores logs to print later

public:
    DFA(bool showDebug = true) : currentState(0), debug(showDebug) {}

    void transition(char c, int nextState, string label = "") {
        if (debug) {
            string charStr = (c == '\n') ? "\\n" : string(1, c);
            string log = charStr + " => State " + to_string(nextState);
            if (!label.empty()) log += " [" + label + "]";
            
            // Save the string to the vector INSTEAD of printing it
            transitionHistory.push_back(log); 
        }
        currentState = nextState;
    }

    void reset() { currentState = 0; }
    int getState() const { return currentState; }

    // This is called AFTER the do-while loop in main finishes
    void printGraph() const {
        if (!debug) return;
        cout << "\n--- DFA Output ---\n";
        for (const string& log : transitionHistory) {
            cout << log << "\n";
        }
    }
};