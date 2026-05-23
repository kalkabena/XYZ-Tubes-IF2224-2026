#include <iostream>
#include <fstream>
#include <sstream>
//#include <filesystem>
#include "lexer/lexer.hpp"
#include "syntax/parsingTree.hpp"
#include "syntax/AST_Tree.hpp"
#include "semantic/SymbolTable.hpp"
#include "semantic/SemanticAnalyzer.hpp"


using namespace std;

int main() {
    string input_path;
    ifstream input_file;

    while (!input_file.is_open()) {
        cout << "Enter input file path (or 'q' to quit): ";
        getline(cin, input_path);
        if (input_path == "q" || input_path == "Q") return 0;

        if (!input_path.empty() && input_path.front() == '"') {
            input_path = input_path.substr(1, input_path.size() - 2);
        }

        input_file.open(input_path);
        if (!input_file.is_open()) cerr << "File not found: " << input_path << endl;
    }

    stringstream buffer;
    buffer << input_file.rdbuf();
    string code = buffer.str();
    input_file.close();

    // filesystem::create_directories("test/output/milestone_1");
    // filesystem::create_directories("test/output/milestone_2");
    // filesystem::create_directories("test/output/milestone_3");

    ofstream out1("test/output/milestone_1.txt");
    if (!out1.is_open()) {
        cerr << "Gagal membuka file output lexer." << endl;
        return 1;
    }

    Lexer lexerForOutput(code);
    Lexer lexerForParser(code);
    Token t;

    cout << "\n--- Lexer Result ---\n" << endl;

    do {
        t = lexerForOutput.getNextToken();
        if (t.type == eof_tok) break;

        string name = getTokenName(t.type);
        string formattedOutput;

        if (t.type == ident || t.type == intcon || t.type == realcon ||
            t.type == charcon || t.type == string_tok || t.type == comment || t.type == unknown_tok) {
            formattedOutput = name + "(" + t.lexeme + ")";
        } else {
            formattedOutput = name;
        }

        out1 << formattedOutput << "\n";
        cout << formattedOutput << endl;

    } while (t.type != eof_tok);

    out1.close();
    cout << "\n--- Lexer selesai. Output: test/output/milestone_1.txt ---\n" << endl;

    cout << "\n--- Parser Result ---\n" << endl;

    try {
        // Fase 1: Parsing (CST Building)
        ParsingTree parser(lexerForParser);
        parser.build();
        parser.printToCLI();
        parser.exportToFile("test/output/milestone_2.txt");
        cout << "\n--- Parser selesai. Output: test/output/milestone_2.txt ---\n" << endl;

        // Fase 2: Transformasi ke AST
        Node* cstRoot = const_cast<Node*>(parser.getRoot()); 
        AST_Tree astBuilder;
        unique_ptr<ASTNode> astRoot = astBuilder.build(cstRoot);

        // Fase 3: Analisis Semantik & Pengisian Symbol Table
        cout << "\n--- Semantic Analysis Result ---\n" << endl;
        
        SymbolTable symTable;
        
        // INSTANSIASI LOGIKA BARU KITA DI SINI
        SemanticAnalyzer analyzer(symTable);
        analyzer.analyze(cstRoot); // Otak analyzer berjalan menelusuri CST dan mengisi data ke symTable
        if (analyzer.hasError()) {
            analyzer.printErrors();
            cout << "\nAnalisis semantik gagal\n";
            return 1;
        }
        
        // Fase 4: Cetak Laporan dan Hasil Akhir
        symTable.printTab(); 
        string milestone3Path = "test/output/milestone_3.txt";
        symTable.exportToFile(milestone3Path, cstRoot, astRoot.get());
        
        cout << "\n--- Semantic Analysis selesai. Output: " << milestone3Path << " ---\n" << endl;

    } catch (const exception& e) {
        cerr << "Compiler Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}