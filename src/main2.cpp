#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "lexer.hpp"
#include "parsingTree.hpp"

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

    filesystem::create_directories("test/milestone_1");
    filesystem::create_directories("test/milestone_2");

    ofstream out1("test/milestone_1/lexer_output.txt");
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
    cout << "\n--- Lexer selesai. Output: test/milestone_1/lexer_output.txt ---\n" << endl;

    cout << "\n--- Parser Result ---\n" << endl;

    try {
        ParsingTree parser(lexerForParser);
        parser.build();
        parser.printToCLI();
        parser.exportToFile("test/milestone_2/syntax_output.txt");
        cout << "\n--- Parser selesai. Output: test/milestone_2/syntax_output.txt ---\n" << endl;
    } catch (const exception& e) {
        cerr << "Parser Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
