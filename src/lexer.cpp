#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <utility> //std::pair
#include <cctype> //std::tolower

enum class State { Esc, StrChar, Comment, Word, Symbol };

const std::array<std::pair<std::string, std::string>, 27> inwords = {{
	{"not", "notsy"},
	{"div", "idiv"},
	{"mod", "imod"},
	{"and", "andsy"},
	{"or", "orsy"},
	{"const", "constsy"},
	{"type", "typesy"},
	{"var", "varsy"},
	{"function", "functionsy"},
	{"procedure", "proceduresy"},
	{"array", "arraysy"},
	{"record", "recordsy"},
	{"program", "programsy"},
	{"begin", "beginsy"},
	{"if", "ifsy"},
	{"case", "casesy"},
	{"repeat", "repeatsy"},
	{"while", "whilesy"},
	{"for", "forsy"},
	{"end", "endsy"},
	{"else", "elsesy"},
	{"until", "untilsy"},
	{"of", "ofsy"},
	{"do", "dosy"},
	{"to", "tosy"},
	{"downto", "downtosy"},
	{"then", "thensy"}
}};

const std::array<std::pair<std::string, std::string>, 19> insymbols = {{
	{"==", "eql"},
	{"<>", "neq"},
	{">=", "geq"},
	{"<=", "leq"},
	{":=", "becomes"},
	{"+", "plus"},
	{"-", "minus"},
	{"*", "times"},
	{"/", "rdiv"},
	{">", "gtr"},
	{"<", "lss"},
	{"(", "lparent"},
	{")", "rparent"},
	{"[", "lbrack"},
	{"]", "rbrack"},
	{",", "comma"},
	{";", "semicolon"},
	{".", "period"},
	{":", "colon"}
}};

bool is_word(char c) {
	if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
		return true;
	}
	return false;
}

bool is_digit(char c) {
	if (c >= '0' && c <= '9') {
		return true;
	}
	return false;
}

void print_word(std::ofstream& output, std::string& token) {
	if (!output.is_open()) {
        std::cerr << "Error opening output file!" << std::endl;
        return;
    }
	
	if (token.empty()) {
		std::cerr << "Error: token is empty!" << std::endl;
        return;
	}
	
	bool is_int = true;
	
	for (char c : token) {
		if (!is_digit(c)) is_int = false;
	}
	
	if (is_int == true) {
		output << "intcon(" << token << ")" << std::endl;
		token.clear();
		return;
	}
	
	bool is_real = true;
	
	for (char c : token) {
		if (!is_digit(c) && c != '.') is_real = false;
	}
	
	if (is_real == true) {
		output << "realcon(" << token << ")" << std::endl;
		token.clear();
		return;
	}
	
	for (char& c : token) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
	
	for (const std::pair<std::string, std::string>& intoken : inwords) {
		if (token == intoken.first) {
			output << intoken.second << std::endl;
			token.clear();
			return;
		}
	}
	output << "ident(" << token << ")" << std::endl;
	token.clear();
}

void print_symbol(std::ofstream& output, std::string& token) {
	if (!output.is_open()) {
        std::cerr << "Error opening output file!" << std::endl;
        return;
    }
	if (token.empty()) {
		std::cerr << "Error: token is empty!" << std::endl;
        return;
	}
	
	int subsize = token.size();
	
	while (subsize > 0) {
		for (const std::pair<std::string, std::string>& intoken : insymbols) {
			if (token.substr(0, subsize) == intoken.first) {
				output << intoken.second << std::endl;
				token = token.substr(subsize, token.size() - subsize);
				return;
			}
		}
		subsize--;
	}
	std::cerr << "Error: Invalid symbol!" << std::endl;
	token.clear();
}

int main() {
	std::string input_file_path;
    std::ifstream input_file;

    while (!input_file.is_open()) {
		std::cout << "Enter absolute input_file path: ";
		std::getline(std::cin, input_file_path);
		if (input_file_path == "q" || input_file_path == "Q") return 0;
		
		if (!input_file_path.empty() && input_file_path.front() == '"' && input_file_path.back() == '"') {
            input_file_path = input_file_path.substr(1, input_file_path.size() - 1);
        }

		input_file.open(input_file_path);
		
		if (!input_file.is_open()) {
			std::cerr << "Unable to open input file " << input_file_path << std::endl;
		}
    }
	
	std::ofstream output_file("lexer_output.txt");
	if (!output_file.is_open()) {
        std::cerr << "Error opening output file!" << std::endl;
        return 1; // Return with an error code
    }
	
	std::string line;
	std::string cur_token;
	State st = State::Word;
    while (std::getline(input_file, line)) { // Read line by line
        //std::cout << line << std::endl;
		for (int i = 0; i < line.length(); i++) {
			if (st == State::Esc) {
				if (line[i] == '\'') {
					cur_token += '\'';
					st = State::StrChar;
					continue;
				} else {
					if (cur_token.size() == 1) {
						output_file << "charcon('" << cur_token << "')\n";
					} else {
						output_file << "string('" << cur_token << "')\n";
					}
					cur_token.clear();
					st = State::Word;
				}
			}
			if (st == State::StrChar) {
				if (line[i] == '\'') {
					st = State::Esc;
				} else {
					cur_token += line[i];
				}
			} else if (st == State::Comment) {
				if (line[i] = '}') {
					if (!cur_token.empty() && cur_token.front() == '*' && cur_token.back() == '*') {
						cur_token = cur_token.substr(1, cur_token.size() - 1);
					}
					output_file << "comment(" << cur_token << ")\n";
					cur_token.clear();
					st = State::Word;
				} else {
					cur_token += line[i];
				}
			} else if (st == State::Word) {
				//std::cout << cur_token << "|" << line[i] << "|" << (!is_word(line[i]) && !cur_token.empty()) << std::endl;
				if (line[i] == '\'') {
					print_word(output_file, cur_token);
					st = State::StrChar;
					continue;
				} else if (line[i] == '{') {
					if (!cur_token.empty()) print_word(output_file, cur_token);
					st = State::Comment;
					continue;
				} else if (line[i] == ' ') {
					if (!cur_token.empty()) print_word(output_file, cur_token);
				} else if (!is_word(line[i])) {
					if (!cur_token.empty()) print_word(output_file, cur_token);
					st = State::Symbol;
				}
				if (line[i] != ' ') cur_token += line[i];
			} else if (st == State::Symbol) {
				if (line[i] == '\'') {
					print_symbol(output_file, cur_token);
					st = State::StrChar;
					continue;
				} else if (line[i] == '{') {
					print_symbol(output_file, cur_token);
					st = State::Comment;
					continue;
				} else if (line[i] == ' ') {
					while (!cur_token.empty()) {
						print_symbol(output_file, cur_token);
					}
				} else if (is_word(line[i])) {
					while (!cur_token.empty()) {
						print_symbol(output_file, cur_token);
					}
					st = State::Word;
				}
				if (line[i] != ' ') cur_token += line[i];
			}
			//std::cout << line[i] << is_word(line[i]) << std::endl;
		}
    }
	if (st == State::StrChar || st == State::Comment) {
		std::cerr << "Error: stale state" << std::endl;
	} else if (st == State::Word) {
		print_word(output_file, cur_token);
	} else if (st == State::Symbol) {
		while (!cur_token.empty()) {
			print_symbol(output_file, cur_token);
		}
	}

    input_file.close(); // Close the input_file (optional, destructor does this automatically)
	output_file.close();
    return 0;
}
