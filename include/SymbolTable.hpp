#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Node.hpp"
#include "AST_Tree.hpp"

enum ObjectClass {
    OBJ_NONE = 0,
    OBJ_CONSTANT,
    OBJ_VARIABLE,
    OBJ_TYPE,
    OBJ_PROCEDURE,
    OBJ_FUNCTION
};

enum Identifier {
    ID_NONE = 0,
    ID_REAL = 26,
    ID_INTEGER = 27,
    ID_CHAR = 28,
    ID_BOOLEAN = 29,
    ID_STRING = 30,
    ID_TRUE = 31,
    ID_FALSE = 32
};

enum DataType {
    TYPE_NONE = 0,
    TYPE_INTEGER,
    TYPE_REAL,
    TYPE_CHAR,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_RECORD
};

struct TabEntry {
    std::string name; 
    int link;        
    ObjectClass obj; 
    DataType type;    
    int ref;         
    int nrm;         
    int lev;         
    int adr;         
};

struct AtabEntry {
    DataType xtyp;    
    DataType etyp;    
    int eref;        
    int low;          
    int high;         
    int elsz;        
    int size;        
};

struct BtabEntry {
    int last;        
    int lpar;         
    int psze;        
    int vsze;      
};

class SymbolTable {
private:
    std::vector<TabEntry> tab;
    std::vector<AtabEntry> atab;
    std::vector<BtabEntry> btab;

    int currentLev = 0;
    int currentBlock = 0;

    DataType resolveTypeFromNode(Node* typeNode) const;
    void traverseNode(Node* node, int lev);

public:
    SymbolTable();
    ~SymbolTable() = default;

    void initializePredefinedIdentifiers();

    int addEntry(const std::string& name, ObjectClass obj, DataType type, int ref, int nrm, int lev, int adr, int link = 0);
    int buildArrayEntry(Node* arrayTypeNode);
    int lookupIndex(const std::string& name) const;

    void buildFromNode(Node* cstRoot);

    void exportToFile(const std::string& filepath, Node* cstRoot, ASTNode* astRoot) const;
    void printTab() const;
};