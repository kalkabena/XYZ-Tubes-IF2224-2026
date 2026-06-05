#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/lexer.hpp"
#include "syntax/parsingTree.hpp"
#include "syntax/AST_Tree.hpp"
#include "semantic/SymbolTable.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include "interpreter/VirtualMachine.hpp"
#include "icg/ICGenerator.hpp"

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
        SemanticAnalyzer analyzer(symTable);
        analyzer.analyze(cstRoot); 
        analyzer.analyzeAST(astRoot.get());
        
        // Error Handling
        if (analyzer.hasError()) {
            analyzer.printErrors();
            cout << "\nAnalisis semantik gagal\n";

            // Generate Output
            ofstream out3("test/output/milestone_3.txt", ios::trunc);
            if (out3.is_open()) {
                for (const auto& err : analyzer.getErrors()) {
                    out3 << err << "\n";
                }
                out3 << "\nAnalisis semantik gagal\n";
                out3.close();
            } else {
                cerr << "Gagal membuka file output milestone 3 untuk penulisan error." << endl;
            }

            return 1;
        }
        
        // Fase 4: Print Output
        symTable.printTab(); 
        string milestone3Path = "test/output/milestone_3.txt";
        symTable.exportToFile(milestone3Path, cstRoot, astRoot.get());
        
        cout << "\n--- Semantic Analysis selesai. Output: " << milestone3Path << " ---\n" << endl;
        // Fase 5: Intermediate Code Generation (ICG)
        cout << "\n--- Intermediate Code Generation Result ---\n" << endl;
        ICGenerator icg(symTable);
        icg.generate(astRoot.get());
        icg.printCode(); // Cetak ke terminal
        
        // Ekspor ICG ke file output Milestone 4
        ofstream out4("test/output/milestone_4.txt");
        if (out4.is_open()) {
            for (const auto& instr : icg.getCode()) {
                out4 << instr.lineNo << " " << instr.op << " " << instr.level << " " << instr.arg << "\n";
            }
            out4.close();
            cout << "\n--- ICG selesai. Output: test/output/milestone_4.txt ---\n" << endl;
        } else {
            cerr << "Gagal membuka file output milestone 4." << endl;
        }

        // Fase 6: Eksekusi Virtual Machine
        cout << "\n--- Virtual Machine Execution ---\n" << endl;
        VirtualMachine vm;
        vm.loadCode(icg.getCode());
        vm.run();
        cout << "\n\n--- Eksekusi Program Selesai ---\n" << endl;

    } catch (const exception& e) {
        cerr << "Compiler Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}