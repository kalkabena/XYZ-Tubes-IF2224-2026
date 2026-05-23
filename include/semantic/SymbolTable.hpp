#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "syntax/ASTNode.hpp"

class Node;     // Forward declaration murni untuk parameter fungsi exportToFile
class ASTNode;  // Forward declaration murni untuk parameter fungsi exportToFile

enum ObjectClass {
    OBJ_NONE = 0,
    OBJ_CONSTANT,
    OBJ_VARIABLE,
    OBJ_TYPE,
    OBJ_PROCEDURE,
    OBJ_FUNCTION
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

    void initializePredefinedIdentifiers();

public:
    SymbolTable();
    ~SymbolTable() = default;

    // API Publik Manajemen Data TAB
    int addEntry(const std::string& name, ObjectClass obj, DataType type, int ref, int nrm, int lev, int adr, int link = 0);
    int lookupIndex(const std::string& name) const;
    TabEntry getTabEntry(int index) const;
    size_t getTabSize() const { return tab.size(); }

    // API Publik Manajemen Data BTAB (Scope)
    int pushNewBlock(int lastIdx);
    void updateCurrentBlockLast(int lastIdx);
    void incrementCurrentBlockVsze(int lastIdx);
    void updateCurrentBlockLpar(int lparCount);
    void resetBlockZero();

    // API Publik Manajemen Data ATAB (Array)
    int buildAtabEntryDirect(DataType xtyp, DataType etyp, int eref, int low, int high, int elsz, int size);

    // Fungsi Cetak Laporan
    void exportToFile(const std::string& filepath, Node* cstRoot, ASTNode* astRoot) const;
    void printTab() const;

    // API Publik Manajemen Data ATAB (Array)
    AtabEntry getAtabEntry(int index) const; // Tambahkan baris ini
};