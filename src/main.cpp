#include "lexer.hpp"    

using namespace std;

int main() {
    string input_file_path;
    ifstream input_file;

    while (!input_file.is_open()) {
        cout << "Enter absolute input_file path: ";
        getline(cin, input_file_path);
        
        if (input_file_path == "q" || input_file_path == "Q") {
            return 0;
        }
        
        if (!input_file_path.empty() && input_file_path.front() == '"' && input_file_path.back() == '"') {
            input_file_path = input_file_path.substr(1, input_file_path.size() - 2);
        }

        input_file.open(input_file_path);
        
        if (!input_file.is_open()) {
            cerr << "Unable to open input file " << input_file_path << endl;
        }
    }
    stringstream buffer;
    buffer << input_file.rdbuf();
    string source_code = buffer.str();
    input_file.close();

    ofstream output_file("lexer_output.txt");
    if (!output_file.is_open()) {
        cerr << "Error opening output file!" << endl;
        return 1;
    }

    Lexer lexer(source_code);
    Token t;

    do {
        t = lexer.getNextToken();

        if (t.type == eof_tok) break;

        string t_name = getTokenName(t.type);

        if (t.type == ident || t.type == intcon || t.type == realcon || 
            t.type == charcon || t.type == string_tok || t.type == comment) {
            output_file << t_name << "(" << t.lexeme << ")\n";
        } 
        else if (t.type == unknown_tok) {
            cerr << "Error: Invalid symbol! (" << t.lexeme << ")" << endl;
            output_file << "unknown_tok(" << t.lexeme << ")\n";
        } 
        else {
            output_file << t_name << "\n";
        }

    } while (t.type != eof_tok);

    output_file.close();

    return 0;
}