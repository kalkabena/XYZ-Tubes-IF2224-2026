#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/lexer.hpp"
#include "syntax/parsingTree.hpp"
#include "syntax/AST_Tree.hpp"
#include "semantic/SymbolTable.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include "interpreter/SafeStackMachine.hpp"
#include "interpreter/RuntimeProtection.hpp"
#include "interpreter/RuntimeProtectionSelfTest.hpp"
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

    ofstream out1("test/output/lexer_report.txt");
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
    cout << "\n--- Lexer selesai. Output: test/output/lexer_report.txt ---\n" << endl;

    cout << "\n--- Parser Result ---\n" << endl;

    try {
        // Fase 1: Parsing (CST Building)
        ParsingTree parser(lexerForParser);
        parser.build();
        parser.printToCLI();
        parser.exportToFile("test/output/syntax_report.txt");
        cout << "\n--- Parser selesai. Output: test/output/syntax_report.txt ---\n" << endl;

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
            ofstream out3("test/output/semantic_report.txt", ios::trunc);
            if (out3.is_open()) {
                for (const auto& err : analyzer.getErrors()) {
                    out3 << err << "\n";
                }
                out3 << "\nAnalisis semantik gagal\n";
                out3.close();
            } else {
                cerr << "Gagal membuka file output semantic_report untuk penulisan error." << endl;
            }

            return 1;
        }
        
        // Fase 4: Print Output
        symTable.printTab(); 
        string milestone3Path = "test/output/semantic_report.txt";
        symTable.exportToFile(milestone3Path, cstRoot, astRoot.get());
        
        cout << "\n--- Semantic Analysis selesai. Output: " << milestone3Path << " ---\n" << endl;
        
        // Fase 5: Intermediate Code Generation (ICG)
        cout << "\n--- Intermediate Code Generation Result ---\n" << endl;
        ICGenerator icg(symTable);
        icg.generate(astRoot.get());
        icg.printCode(); // Cetak ke terminal
        
        // Ekspor ICG ke file output Milestone 4
        ofstream out4("test/output/icg_intepreter_report.txt");
        if (out4.is_open()) {
            for (const auto& instr : icg.getCode()) {
                out4 << instr.lineNo << " " << instr.op << " " << instr.level << " " << instr.arg << "\n";
            }
            out4.close();
            cout << "\n--- ICG selesai. Output: test/output/icg_intepreter_report.txt ---\n" << endl;
        } else {
            cerr << "Gagal membuka file output milestone 4." << endl;
        }

        // Vulnerability
        cout << "\n--- Runtime Protection Self Test ---\n" << endl;
        auto testResults = interpreter::runVulnerabilitySelfTests();
        for (const auto& result : testResults) {
            cout << "Test: " << result.name << " - " 
                 << (result.passed ? "PASSED" : "FAILED") << "\n";
            if (!result.passed) {
                cout << "  Message: " << result.message << "\n";
            }
        }
        interpreter::writeVulnerabilityReport("test/output/vulnerability_report.txt");
        cout << "\nSelf test report written to test/output/vulnerability_report.txt\n" << endl;

        cout << "\n--- Safe Stack Machine Execution ---\n" << endl;
        
        std::vector<interpreter::Instruction> safeInstructions;
        for (const auto& tac : icg.getCode()) {
            interpreter::OpCode opCode;
            if (tac.op == "LIT") opCode = interpreter::OpCode::LIT;
            else if (tac.op == "LOD") opCode = interpreter::OpCode::LOD;
            else if (tac.op == "STO") opCode = interpreter::OpCode::STO;
            else if (tac.op == "CAL") opCode = interpreter::OpCode::CAL;
            else if (tac.op == "INT") opCode = interpreter::OpCode::INT;
            else if (tac.op == "JMP") opCode = interpreter::OpCode::JMP;
            else if (tac.op == "JPC") opCode = interpreter::OpCode::JPC;
            else if (tac.op == "OPR") opCode = interpreter::OpCode::OPR;
            else if (tac.op == "RET") opCode = interpreter::OpCode::RET;
            else if (tac.op == "LODI") opCode = interpreter::OpCode::LODI; 
            else if (tac.op == "STOI") opCode = interpreter::OpCode::STOI; 
            else throw runtime_error("Unknown OpCode: " + tac.op);
            
            safeInstructions.push_back(interpreter::Instruction(opCode, tac.level, tac.arg));
        }

        interpreter::SafeStackMachine safeVm(safeInstructions);
        safeVm.run();
        cout << "\n\n--- Eksekusi Program Selesai ---\n" << endl;

    } catch (const exception& e) {
        cerr << "Compiler Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}