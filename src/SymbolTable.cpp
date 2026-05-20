#include "SymbolTable.hpp"
#include <stdexcept>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace std;
static string extractRawValue(const string& formattedLexeme) {
    size_t start = formattedLexeme.find('(');
    size_t end = formattedLexeme.rfind(')');
    if (start != string::npos && end != string::npos && start < end) {
        return formattedLexeme.substr(start + 1, end - start - 1);
    }
    return formattedLexeme;
}

SymbolTable::SymbolTable() {
    atab.push_back({}); 
    btab.push_back({}); 
    initializePredefinedIdentifiers();
}

void SymbolTable::initializePredefinedIdentifiers() {
    for (int i = 0; i < 26; ++i) {
        tab.push_back({"", 0, OBJ_NONE, TYPE_NONE, 0, 0, 0, 0});
    }

    tab.push_back({"real", 0, OBJ_TYPE, TYPE_REAL, 0, 1, 0, 0});
    tab.push_back({"integer", 0, OBJ_TYPE, TYPE_INTEGER, 0, 1, 0, 0});
    tab.push_back({"char", 0, OBJ_TYPE, TYPE_CHAR, 0, 1, 0, 0});
    tab.push_back({"boolean", 0, OBJ_TYPE, TYPE_BOOLEAN, 0, 1, 0, 0});
    tab.push_back({"string", 0, OBJ_TYPE, TYPE_STRING, 0, 1, 0, 0});
    tab.push_back({"true", 0, OBJ_CONSTANT, TYPE_BOOLEAN, 0, 1, 0, 1});
    tab.push_back({"false", 0, OBJ_CONSTANT, TYPE_BOOLEAN, 0, 1, 0, 0});
}

int SymbolTable::addEntry(const string& name, ObjectClass obj, DataType type, int ref, int nrm, int lev, int adr, int link) {
    TabEntry entry;
    entry.name = name;
    entry.link = link;
    entry.obj = obj;
    entry.type = type;
    entry.ref = ref;
    entry.nrm = nrm;
    entry.lev = lev;
    entry.adr = adr;
    
    tab.push_back(entry);
    return tab.size() - 1;
}

int SymbolTable::buildArrayEntry(Node* arrayTypeNode) {
    if (!arrayTypeNode || arrayTypeNode->type != ARRAY_TYPE) return 0;

    Node* boundsNode = arrayTypeNode->children[2].get(); 
    Node* elementTypeNode = arrayTypeNode->children[5].get();

    int lowBound = 0;
    int highBound = 0;
    DataType indexType = TYPE_INTEGER; 

    if (boundsNode->type == RANGE) {
        string lowStr = extractRawValue(boundsNode->children[0]->lexeme);
        string highStr = extractRawValue(boundsNode->children[3]->lexeme);
        
        try {
            lowBound = stoi(lowStr.empty() ? "0" : lowStr);
            highBound = stoi(highStr.empty() ? "0" : highStr);
        } catch (...) {
            lowBound = 0;
            highBound = 0;
        }
    }

    DataType elType = resolveTypeFromNode(elementTypeNode);
    int elementSize = 1; 
    int totalSize = (highBound - lowBound + 1) * elementSize;

    AtabEntry entry;
    entry.xtyp = indexType; 
    entry.etyp = elType;
    entry.eref = 0; 
    entry.low = lowBound;
    entry.high = highBound;
    entry.elsz = elementSize;
    entry.size = totalSize > 0 ? totalSize : 0;

    atab.push_back(entry);
    return atab.size() - 1; 
}

int SymbolTable::lookupIndex(const string& name) const {
    for (int i = tab.size() - 1; i >= 26; --i) {
        if (tab[i].name == name) {
            return i;
        }
    }
    return 0; 
}

DataType SymbolTable::resolveTypeFromNode(Node* typeNode) const {
    if (!typeNode) return TYPE_NONE;

    if (!typeNode->children.empty()) {
        if (typeNode->children[0]->type == ARRAY_TYPE) return TYPE_ARRAY;
        if (typeNode->children[0]->type == RECORD_TYPE) return TYPE_RECORD;
    }
    string typeName = extractRawValue(typeNode->lexeme);
    

    if (typeName == "integer") return TYPE_INTEGER;
    if (typeName == "real")    return TYPE_REAL;
    if (typeName == "char")    return TYPE_CHAR;
    if (typeName == "boolean") return TYPE_BOOLEAN;
    if (typeName == "string")  return TYPE_STRING;
    if (typeName.empty() && !typeNode->children.empty()) {
        typeName = extractRawValue(typeNode->children[0]->lexeme);
    }

    if (typeName == "array")  return TYPE_ARRAY;
    if (typeName == "record") return TYPE_RECORD;

    int index = lookupIndex(typeName);
    if (index > 0 && tab[index].obj == OBJ_TYPE) {
        return tab[index].type; 
    }

    return TYPE_NONE;
}

void SymbolTable::buildFromNode(Node* cstRoot) {
    currentLev = 0;
    currentBlock = 0;
    
    btab[0].last = 0; 
    btab[0].lpar = 0;
    btab[0].psze = 0;
    btab[0].vsze = 0;

    traverseNode(cstRoot, currentLev);
}

void SymbolTable::traverseNode(Node* node, int lev) {
    if (!node) return;

    switch (node->type) {
        case PROGRAM: {
            string progName = "unknown";
            if (!node->children.empty() && node->children[0]->type == PROGRAM_HEADER) {
                if (node->children[0]->children.size() > 1) {
                    progName = extractRawValue(node->children[0]->children[1]->lexeme);
                }
            }
            
            int idx = addEntry(progName, OBJ_PROCEDURE, TYPE_NONE, 0, 1, lev, 0);

            int prevBlock = currentBlock;
            currentBlock = btab.size();
            btab.push_back({0, 0, 0, 0});
            btab[currentBlock].last = idx;

            for (auto& child : node->children) {
                traverseNode(child.get(), lev);
            }

            currentBlock = prevBlock;
            break;
        }
        case CONST_DECLARATION: {
            size_t i = 1;
            while (i + 2 < node->children.size()) {
                Node* idNode = node->children[i].get();
                Node* constNode = node->children[i+2].get();
                
                if (idNode->type == TOKEN_NODE) {
                    string constName = extractRawValue(idNode->lexeme);
                    DataType constType = TYPE_NONE;
                    
                    if (!constNode->children.empty()) {
                        string valLexeme = constNode->children.back()->lexeme;
                        if (valLexeme.find("intcon") != string::npos) constType = TYPE_INTEGER;
                        else if (valLexeme.find("realcon") != string::npos) constType = TYPE_REAL;
                        else if (valLexeme.find("charcon") != string::npos) constType = TYPE_CHAR;
                        else if (valLexeme.find("string_tok") != string::npos) constType = TYPE_STRING;
                    }
                    
                    int idx = addEntry(constName, OBJ_CONSTANT, constType, 0, 1, lev, 0);
                    if (currentBlock < btab.size()) {
                        btab[currentBlock].last = idx;
                    }
                }
                i += 4;
            }
            break;
        }
        case TYPE_DECLARATION: {
            size_t i = 1;
            while (i + 2 < node->children.size()) {
                Node* idNode = node->children[i].get();
                Node* typeNode = node->children[i+2].get();
                
                if (idNode->type == TOKEN_NODE) {
                    string typeName = extractRawValue(idNode->lexeme);
                    DataType resolvedType = resolveTypeFromNode(typeNode);
                    int typeRef = 0;

                    Node* actualType = typeNode;
                    if (!typeNode->children.empty()) {
                        actualType = typeNode->children[0].get();
                    }

                    if (resolvedType == TYPE_ARRAY && actualType->type == ARRAY_TYPE) {
                        typeRef = buildArrayEntry(actualType);
                    }

                    int idx = addEntry(typeName, OBJ_TYPE, resolvedType, typeRef, 1, lev, 0);
                    if (currentBlock < btab.size()) {
                        btab[currentBlock].last = idx;
                    }
                }
                i += 4;
            }
            break;
        }
        case VAR_DECLARATION: {
            size_t i = 1; 
            while (i + 2 < node->children.size()) {
                Node* idListNode = node->children[i].get();
                Node* typeNode = node->children[i+2].get(); 
                
                DataType resolvedType = resolveTypeFromNode(typeNode);
                int typeRef = 0;
                
                Node* actualType = typeNode;
                if (!typeNode->children.empty()) {
                    actualType = typeNode->children[0].get();
                }

                if (resolvedType == TYPE_ARRAY && actualType->type == ARRAY_TYPE) {
                    typeRef = buildArrayEntry(actualType);
                } else if (resolvedType == TYPE_ARRAY || resolvedType == TYPE_RECORD) {
                    string tName = extractRawValue(typeNode->lexeme);
                    if (tName.empty() && !typeNode->children.empty()) {
                        tName = extractRawValue(typeNode->children[0]->lexeme);
                    }
                    int tIdx = lookupIndex(tName);
                    if (tIdx > 0 && tab[tIdx].obj == OBJ_TYPE) {
                        typeRef = tab[tIdx].ref;
                    }
                }

                for (auto& identNode : idListNode->children) {
                    if (identNode->type == TOKEN_NODE && identNode->lexeme.find("ident") != string::npos) {
                        string varName = extractRawValue(identNode->lexeme);
                        
                        int idx = addEntry(varName, OBJ_VARIABLE, resolvedType, typeRef, 1, lev, 0);
                        if (currentBlock < btab.size()) {
                            btab[currentBlock].last = idx;
                            btab[currentBlock].vsze += 1;
                        }
                    }
                }
                i += 4; 
            }
            break;
        }
        case FUNCTION_DECLARATION:
        case PROCEDURE_DECLARATION: {
            string subName = extractRawValue(node->children[1]->lexeme);
            ObjectClass objClass = (node->type == FUNCTION_DECLARATION) ? OBJ_FUNCTION : OBJ_PROCEDURE;
            DataType retType = TYPE_NONE;

            if (objClass == OBJ_FUNCTION) {
                for (size_t i = 0; i < node->children.size(); ++i) {
                    if (node->children[i]->lexeme == "colon") {
                        retType = resolveTypeFromNode(node->children[i+1].get());
                        break;
                    }
                }
            }

            int procIdx = addEntry(subName, objClass, retType, 0, 1, lev, 0);
            if (currentBlock < btab.size()) {
                btab[currentBlock].last = procIdx;
            }

            int prevBlock = currentBlock;
            currentBlock = btab.size();
            btab.push_back({0, 0, 0, 0});
            btab[currentBlock].last = procIdx;

            int paramCount = 0;
            for (auto& child : node->children) {
                if (child->type == FORMAL_PARAMETER_LIST) {
                    for (auto& pGroup : child->children) {
                        if (pGroup->type == PARAMETER_GROUP) {
                            Node* identList = pGroup->children[0].get();
                            Node* typeNode = pGroup->children[2].get();
                            DataType paramType = resolveTypeFromNode(typeNode);
                            
                            int pRef = 0;
                            Node* pActualType = typeNode;
                            if (!typeNode->children.empty()) pActualType = typeNode->children[0].get();
                            if (paramType == TYPE_ARRAY && pActualType->type == ARRAY_TYPE) {
                                pRef = buildArrayEntry(pActualType);
                            } else if (paramType == TYPE_ARRAY || paramType == TYPE_RECORD) {
                                string pTName = extractRawValue(typeNode->lexeme);
                                if (pTName.empty() && !typeNode->children.empty()) pTName = extractRawValue(typeNode->children[0]->lexeme);
                                int pTIdx = lookupIndex(pTName);
                                if (pTIdx > 0 && tab[pTIdx].obj == OBJ_TYPE) pRef = tab[pTIdx].ref;
                            }

                            for (auto& leaf : identList->children) {
                                if (leaf->lexeme.find("ident(") == 0) {
                                    string paramName = extractRawValue(leaf->lexeme);
                                    int idx = addEntry(paramName, OBJ_VARIABLE, paramType, pRef, 1, lev + 1, 0);
                                    btab[currentBlock].last = idx;
                                    btab[currentBlock].vsze += 1;
                                    paramCount++;
                                }
                            }
                        }
                    }
                }
            }
            btab[currentBlock].lpar = paramCount;

            for (auto& child : node->children) {
                if (child->type == BLOCK) traverseNode(child.get(), lev + 1);
            }

            currentBlock = prevBlock;
            break;
        }
        default: {
            for (auto& child : node->children) {
                traverseNode(child.get(), lev);
            }
            break;
        }
    }
}

void SymbolTable::exportToFile(const string& filepath, Node* cstRoot, ASTNode* astRoot) const {
    (void)cstRoot;

    ofstream outFile(filepath);
    if (!outFile.is_open()) {
        throw runtime_error("File Error: Cannot open " + filepath);
    }

    auto printBoth = [&](const string& text) {
        outFile << text;
        cout << text;
    };

    string tabHeader = "--- SYMBOL TABLE (TAB) ---\n"
                            "idx  id             obj         type     ref  nrm  lev  adr  link\n"
                            "-----------------------------------------------------------------\n";
    printBoth(tabHeader);

    for (size_t i = 33; i < tab.size(); ++i) {
        stringstream ss;
        ss << left 
           << setw(5) << i 
           << setw(15) << (tab[i].name.empty() ? "-" : tab[i].name)
           << setw(12) << tab[i].obj 
           << setw(8) << tab[i].type 
           << setw(5) << tab[i].ref 
           << setw(5) << tab[i].nrm 
           << setw(5) << tab[i].lev 
           << setw(5) << tab[i].adr 
           << tab[i].link << "\n";
        printBoth(ss.str());
    }
    printBoth("\n");

    string atabHeader = "--- ARRAY TABLE (ATAB) ---\n"
                             "idx  xtyp   etyp   eref   low    high   elsz   size\n"
                             "---------------------------------------------------\n";
    printBoth(atabHeader);

    for (size_t i = 1; i < atab.size(); ++i) {
        stringstream ss;
        ss << left 
           << setw(5) << i 
           << setw(7) << atab[i].xtyp 
           << setw(7) << atab[i].etyp 
           << setw(7) << atab[i].eref 
           << setw(7) << atab[i].low 
           << setw(7) << atab[i].high 
           << setw(7) << atab[i].elsz 
           << atab[i].size << "\n";
        printBoth(ss.str());
    }
    printBoth("\n");

    string btabHeader = "--- BLOCK TABLE (BTAB) ---\n"
                             "idx  last  lpar  psze  vsze\n"
                             "---------------------------\n";
    printBoth(btabHeader);

    for (size_t i = 0; i < btab.size(); ++i) {
        stringstream ss;
        ss << left 
           << setw(5) << i 
           << setw(6) << btab[i].last 
           << setw(6) << btab[i].lpar 
           << setw(6) << btab[i].psze 
           << btab[i].vsze << "\n";
        printBoth(ss.str());
    }
    printBoth("\n");

    printBoth("--- DECORATED AST ---\n");
    if (astRoot) {
        astRoot->print(outFile, "", true);
        astRoot->print(cout, "", true);
    } else {
        printBoth("AST tidak terbentuk (Cek logika build di AST_Tree.cpp).\n");
    }

    outFile.close();
}

void SymbolTable::printTab() const {
    cout << "\n--- SYMBOL TABLE (TAB) ---\n";
    cout << left 
              << setw(5) << "idx" 
              << setw(15) << "id" 
              << setw(12) << "obj" 
              << setw(8) << "type" 
              << setw(5) << "ref" 
              << setw(5) << "nrm" 
              << setw(5) << "lev" 
              << setw(5) << "adr" 
              << "link\n";
    cout << string(65, '-') << "\n";
    cout << "...  (reserved words 0-32)\n";

    for (size_t i = 33; i < tab.size(); ++i) {
        cout << left 
                  << setw(5) << i 
                  << setw(15) << (tab[i].name.empty() ? "-" : tab[i].name)
                  << setw(12) << tab[i].obj 
                  << setw(8) << tab[i].type 
                  << setw(5) << tab[i].ref 
                  << setw(5) << tab[i].nrm 
                  << setw(5) << tab[i].lev 
                  << setw(5) << tab[i].adr 
                  << tab[i].link << "\n";
    }
    cout << "\n";

    cout << "--- ARRAY TABLE (ATAB) ---\n";
    cout << left 
              << setw(5) << "idx" 
              << setw(7) << "xtyp" 
              << setw(7) << "etyp" 
              << setw(7) << "eref" 
              << setw(7) << "low" 
              << setw(7) << "high" 
              << setw(7) << "elsz" 
              << "size\n";
    cout << string(51, '-') << "\n";

    for (size_t i = 1; i < atab.size(); ++i) {
        cout << left 
                  << setw(5) << i 
                  << setw(7) << atab[i].xtyp 
                  << setw(7) << atab[i].etyp 
                  << setw(7) << atab[i].eref 
                  << setw(7) << atab[i].low 
                  << setw(7) << atab[i].high 
                  << setw(7) << atab[i].elsz 
                  << atab[i].size << "\n";
    }
    cout << "\n";

    cout << "--- BLOCK TABLE (BTAB) ---\n";
    cout << left 
              << setw(5) << "idx" 
              << setw(6) << "last" 
              << setw(6) << "lpar" 
              << setw(6) << "psze" 
              << "vsze\n";
    cout << string(27, '-') << "\n";

    for (size_t i = 0; i < btab.size(); ++i) {
        cout << left 
                  << setw(5) << i 
                  << setw(6) << btab[i].last 
                  << setw(6) << btab[i].lpar 
                  << setw(6) << btab[i].psze 
                  << btab[i].vsze << "\n";
    }
    cout << "\n";
}