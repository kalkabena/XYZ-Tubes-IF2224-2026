#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.hpp"

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
    ofstream out("test/lexer_output.txt");
    if (!out.is_open()) {
        cerr << "Gagal membuka file output." << endl;
        return 1;
    }

    Lexer lexer(code);
    Token t;

    cout << "\n--- Hasil Analisis Leksikal ---\n" << endl;

    do {
        t = lexer.getNextToken();
        if (t.type == eof_tok) break;

        string name = getTokenName(t.type);
        string formattedOutput;

        if (t.type == ident || t.type == intcon || t.type == realcon || 
            t.type == charcon || t.type == string_tok || t.type == comment) {
            formattedOutput = name + "(" + t.lexeme + ")";
        } else {
            formattedOutput = name;
        }

        out << formattedOutput << "\n";
        cout << formattedOutput << endl;

    } while (t.type != eof_tok);

    cout << "\n--- Selesai. Hasil disimpan di test/lexer_output.txt ---\n" << endl;

    out.close();
    return 0;
}