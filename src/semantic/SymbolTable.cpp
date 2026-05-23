#include "semantic/SymbolTable.hpp"
#include <stdexcept>
#include <iomanip>
#include <fstream>
#include <sstream>

SymbolTable::SymbolTable() {
    atab.push_back({}); // Indeks ATAB dimulai dari 1 secara standar
    btab.push_back({}); // Blok 0 untuk predefined identifier layer
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

int SymbolTable::addEntry(const std::string& name, ObjectClass obj, DataType type, int ref, int nrm, int lev, int adr, int link) {
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

int SymbolTable::lookupIndex(const std::string& name) const {
    for (int i = tab.size() - 1; i >= 26; --i) {
        if (tab[i].name == name) {
            return i;
        }
    }
    return 0; 
}

TabEntry SymbolTable::getTabEntry(int index) const {
    if (index < 0 || index >= static_cast<int>(tab.size())) {
        throw std::out_of_range("SymbolTable Error: Index TAB di luar jangkauan.");
    }
    return tab[index];
}

int SymbolTable::pushNewBlock(int lastIdx) {
    btab.push_back({lastIdx, 0, 0, 0});
    return btab.size() - 1;
}

void SymbolTable::updateCurrentBlockLast(int lastIdx) {
    if (!btab.empty()) btab.back().last = lastIdx;
}

void SymbolTable::incrementCurrentBlockVsze(int lastIdx) {
    if (!btab.empty()) {
        btab.back().last = lastIdx;
        btab.back().vsze += 1;
    }
}

void SymbolTable::updateCurrentBlockLpar(int lparCount) {
    if (!btab.empty()) btab.back().lpar = lparCount;
}

int SymbolTable::buildAtabEntryDirect(DataType xtyp, DataType etyp, int eref, int low, int high, int elsz, int size) {
    AtabEntry entry = {xtyp, etyp, eref, low, high, elsz, size};
    atab.push_back(entry);
    return atab.size() - 1;
}

void SymbolTable::resetBlockZero() {
    if (!btab.empty()) btab[0] = {0, 0, 0, 0};
}

void SymbolTable::exportToFile(const std::string& filepath, Node* cstRoot, ASTNode* astRoot) const {
    (void)cstRoot; 

    std::ofstream outFile(filepath);
    if (!outFile.is_open()) {
        throw std::runtime_error("File Error: Cannot open " + filepath);
    }

    auto printBoth = [&](const std::string& text) {
        outFile << text;
        std::cout << text;
    };

    std::string tabHeader = "--- SYMBOL TABLE (TAB) ---\n"
                            "idx  id             obj         type     ref  nrm  lev  adr  link\n"
                            "-----------------------------------------------------------------\n";
    printBoth(tabHeader);

    for (size_t i = 33; i < tab.size(); ++i) {
        std::stringstream ss;
        ss << std::left 
           << std::setw(5) << i 
           << std::setw(15) << (tab[i].name.empty() ? "-" : tab[i].name)
           << std::setw(12) << tab[i].obj 
           << std::setw(8) << tab[i].type 
           << std::setw(5) << tab[i].ref 
           << std::setw(5) << tab[i].nrm 
           << std::setw(5) << tab[i].lev 
           << std::setw(5) << tab[i].adr 
           << tab[i].link << "\n";
        printBoth(ss.str());
    }
    printBoth("\n");

    std::string atabHeader = "--- ARRAY TABLE (ATAB) ---\n"
                             "idx  xtyp   etyp   eref   low    high   elsz   size\n"
                             "---------------------------------------------------\n";
    printBoth(atabHeader);

    for (size_t i = 1; i < atab.size(); ++i) {
        std::stringstream ss;
        ss << std::left 
           << std::setw(5) << i 
           << std::setw(7) << atab[i].xtyp 
           << std::setw(7) << atab[i].etyp 
           << std::setw(7) << atab[i].eref 
           << std::setw(7) << atab[i].low 
           << std::setw(7) << atab[i].high 
           << std::setw(7) << atab[i].elsz 
           << atab[i].size << "\n";
        printBoth(ss.str());
    }
    printBoth("\n");

    std::string btabHeader = "--- BLOCK TABLE (BTAB) ---\n"
                             "idx  last  lpar  psze  vsze\n"
                             "---------------------------\n";
    printBoth(btabHeader);

    for (size_t i = 0; i < btab.size(); ++i) {
        std::stringstream ss;
        ss << std::left 
           << std::setw(5) << i 
           << std::setw(6) << btab[i].last 
           << std::setw(6) << btab[i].lpar 
           << std::setw(6) << btab[i].psze 
           << btab[i].vsze << "\n";
        printBoth(ss.str());
    }
    printBoth("\n");

    printBoth("--- DECORATED AST ---\n");
    if (astRoot) {
        astRoot->print(outFile, "", true);
        astRoot->print(std::cout, "", true);
    } else {
        printBoth("AST tidak terbentuk (Cek logika build di AST_Tree.cpp).\n");
    }

    outFile.close();
}

void SymbolTable::printTab() const {
    std::cout << "\n--- SYMBOL TABLE (TAB) ---\n";
    std::cout << std::left 
              << std::setw(5) << "idx" 
              << std::setw(15) << "id" 
              << std::setw(12) << "obj" 
              << std::setw(8) << "type" 
              << std::setw(5) << "ref" 
              << std::setw(5) << "nrm" 
              << std::setw(5) << "lev" 
              << std::setw(5) << "adr" 
              << "link\n";
    std::cout << std::string(65, '-') << "\n";
    std::cout << "...  (reserved words 0-32)\n";

    for (size_t i = 33; i < tab.size(); ++i) {
        std::cout << std::left 
                  << std::setw(5) << i 
                  << std::setw(15) << (tab[i].name.empty() ? "-" : tab[i].name)
                  << std::setw(12) << tab[i].obj 
                  << std::setw(8) << tab[i].type 
                  << std::setw(5) << tab[i].ref 
                  << std::setw(5) << tab[i].nrm 
                  << std::setw(5) << tab[i].lev 
                  << std::setw(5) << tab[i].adr 
                  << tab[i].link << "\n";
    }
    std::cout << "\n";

    std::cout << "--- ARRAY TABLE (ATAB) ---\n";
    std::cout << std::left 
              << std::setw(5) << "idx" 
              << std::setw(7) << "xtyp" 
              << std::setw(7) << "etyp" 
              << std::setw(7) << "eref" 
              << std::setw(7) << "low" 
              << std::setw(7) << "high" 
              << std::setw(7) << "elsz" 
              << "size\n";
    std::cout << std::string(51, '-') << "\n";

    for (size_t i = 1; i < atab.size(); ++i) {
        std::cout << std::left 
                  << std::setw(5) << i 
                  << std::setw(7) << atab[i].xtyp 
                  << std::setw(7) << atab[i].etyp 
                  << std::setw(7) << atab[i].eref 
                  << std::setw(7) << atab[i].low 
                  << std::setw(7) << atab[i].high 
                  << std::setw(7) << atab[i].elsz 
                  << atab[i].size << "\n";
    }
    std::cout << "\n";

    std::cout << "--- BLOCK TABLE (BTAB) ---\n";
    std::cout << std::left 
              << std::setw(5) << "idx" 
              << std::setw(6) << "last" 
              << std::setw(6) << "lpar" 
              << std::setw(6) << "psze" 
              << "vsze\n";
    std::cout << std::string(27, '-') << "\n";

    for (size_t i = 0; i < btab.size(); ++i) {
        std::cout << std::left 
                  << std::setw(5) << i 
                  << std::setw(6) << btab[i].last 
                  << std::setw(6) << btab[i].lpar 
                  << std::setw(6) << btab[i].psze 
                  << btab[i].vsze << "\n";
    }
    std::cout << "\n";
}