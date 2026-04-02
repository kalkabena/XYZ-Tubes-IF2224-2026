# Lexical Analysis

## Identitas Kelompok:
### Nama Kelompok: Empty String
### Anggota Kelompok :
* #### 13524114 Mirza Tsabita Wafa'ana
* #### 13524119 Nathanael Shane Bennet
* #### 13524130 Faris Wirakusuma Triawan
* #### 13524144 Jonathan Harijadi

## Deskripsi Program:
Program dibuat menggunakan bahasa C++ dan implementasi lexer dan dfa terdapat pada file file berikut: lexer.cpp dan hpp token.hpp dan dfa_graph.hpp.
### 1. File token.hpp
File ini bertugas sebagai definisi struktur data fundamental yang akan menjadi output dari lexer dan input bagi parser. Didalam file ini terdapat enum yang berisikan token-token yang nantinya akan dibaca oleh lexical analyzer pemanggilan token dapat dilakukan melalui struct token.

Berikut adalah implemensi enum token:
```c++
enum TokenType {
   // Literals & Identifiers
   intcon, realcon, charcon, string_tok, ident,
  
   // Operators
   notsy, plus_tok, minus_tok, times, idiv, rdiv, imod, andsy, orsy,
   eql, neq, gtr, geq, lss, leq, becomes,
  
   // Delimiters & Punctuation
   lparent, rparent, lbrack, rbrack, comma, semicolon, period, colon,
  
   // Keywords
   constsy, typesy, varsy, functionsy, proceduresy, arraysy, recordsy,
   programsy, beginsy, ifsy, casesy, repeatsy, whilesy, forsy, endsy,
   elsesy, untilsy, ofsy, dosy, tosy, downtosy, thensy,
  
   // Special
   comment, eof_tok, unknown_tok
}
```

Berikut adalah implementasi struct token:

```c++
struct Token {
   TokenType type;
   std::string lexeme;
};
```
### 2. File Lexer.hpp dan Lexer.
Kedua File ini bertugas sebagai lexical analyzer utama dalam program ini. Menggunakan pola arsitektur deklarasi dan implementasi, kedua file ini bekerja secara terintegrasi sebagai mesin pembaca kode sumber yang memvalidasi teks mentah dan mengonversinya menjadi aliran token.

Didalam lexer.cpp terdapat switch pada methode nextToken() disinilah letak DFA nya diimplementasi.Dalam DFA formal, transisi ditentukan oleh State saat ini dan Karakter input. getNextToken() selalu memulai dari STATE_START (State 0), blok switch berfungsi sebagai pemilih jalur pertama berdasarkan karakter yang dibaca.Setiap case dalam switch mewakili sebuah busur (panah)/transisi DFA yang keluar dari STATE_START menuju state berikutnya.didalam case/transisi juga dapat divisualisai via CLI namun tidak digunakan saat ini.

```c++
switch (current) {
       case ';':
           dfa.transition(';', STATE_SEMICOLON);
           return {semicolon, ";"};
       case '.':
           dfa.transition('.', STATE_PERIOD);
           return {period, "."};
       case ':':
           dfa.transition(':', STATE_COLON, "Goten: Colon");
           if (pos < source.length() && source[pos] == '=') {
               char next = source[pos++];
               dfa.transition(next, STATE_BECOMES, "Goten: Becomes");
               return {becomes, ":="};
           }
           return {colon, ":"};
       case '+':
           dfa.transition('+', STATE_PLUS);
           return {plus_tok, "+"};
      // Dan seterusnya …..
       default:
           dfa.transition(current, STATE_UNKNOWN);
           return {unknown_tok, std::string(1, current)};
   }

```


## Requirements:
* MakeFile

### Cara Instalasi dan Penggunaan Program:
#### Kompilasi :

Gunakan Makefile yang tersedia untuk mengelola build secara otomatis:
```make
# Untuk Menjalankan program
make run
# Untuk Membersikan bin dan exe lama
make clean
```
Jika tidak menggunakan Makefile:
```bash
g++ -std=c++17 -Iinclude src/lexer.cpp src/main.cpp -o bin/compiler
```

### Pembagian Tugas:
