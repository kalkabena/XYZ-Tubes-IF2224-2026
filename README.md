# ARION COMPILER

 Identitas Kelompok:
### Nama Kelompok: Empty String
### Anggota Kelompok :
* #### 13524114 Mirza Tsabita Wafa'ana
* #### 13524119 Nathanael Shane Bennet
* #### 13524130 Faris Wirakusuma Triawan
* #### 13524144 Jonathan Harijadi

## Deskripsi Program:
Program dibuat menggunakan bahasa C++ dan implementasi lexer dan dfa terdapat pada file file berikut: lexer.cpp dan hpp token.hpp dan dfa_graph.hpp.

###  ***A. LEXER ANALIZER <Milestone_1>*** 


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
###  ***B. SYNTAX ANALIZER<Milestone_2>***

### 1. File Node.hpp

Kelas Node berfungsi sebagai struktur data fundamental dalam pembentukan Parse Tree (Pohon Sintaksis). Secara teknis, kelas ini bertugas menyimpan identitas hierarki tata bahasa kompilator, menyimpan nilai aktual token dari Lexer untuk leaf node, serta mengelola relasi dan kepemilikan memori terhadap cabang-cabang turunannya secara terpusat menggunakan smart pointers C++.

Selain itu, file ini membungkus metode utilitas printDFS untuk mengeksekusi penelusuran pohon secara Depth-First Search guna mencetak visualisasi hierarki sintaksis secara linier ke Command Line Interface (CLI) atau file log eksternal.

#### a. NodeType:
```cpp
enum NodeType {
    TOKEN_NODE,
    PROGRAM,
    PROGRAM_HEADER,
    // type non-terminal lainnya (41 buah)
    ADDITIVE_OPERATOR,
    MULTIPLICATIVE_OPERATOR
};

```
Kumpulan enumerasi ini memetakan seluruh aturan produksi Context-Free Grammar kompilator. Terdapat 44 nilai enum; satu nilai (TOKEN_NODE) didedikasikan sebagai pengenal generik untuk semua node terminal (daun/token mentah), sementara 43 nilai sisanya merupakan representasi node non-terminal mulai dari struktur makro (PROGRAM) hingga tingkat mikro (FACTOR).

#### b. NodeClass:
```cpp
class Node {
private:
public:
    NodeType type;
    string lexeme;
    vector<std::unique_ptr<Node>> children;
    Node(NodeType t) : type(t), lexeme("") {}
    Node(NodeType t, std::string l) : type(t), lexeme(l) {}
    ~Node() = default;
    void addChild(unique_ptr<Node> child);
    void printDFS(std::ostream& os, std::string prefix = "", bool isLast = true, bool isRoot = true) const;
};
```
- type: Menyimpan identitas struktural node saat ini berdasarkan enumerasi NodeType.

- lexeme: Variabel penyimpan nilai string aktual (hanya diisi jika node tersebut adalah TOKEN_NODE).

- children: Vektor dinamis yang menampung node turunan. Penggunaan <code>std::unique_ptr<Node> </code>menjamin kepemilikan memori eksklusif; ketika root atau parent node dihapus (melalui destruktor ~Node() = default), seluruh cabang turunannya akan dibersihkan dari heap memory secara otomatis tanpa risiko memory leak.

- printDFS: Fungsi rekursif konstan yang menelusuri graf pohon mulai dari induk hingga daun. Metode ini merekayasa string prefix (seperti ├── dan └──) untuk merender representasi visual 2D ke format output stream C++ (std::cout atau objek file std::ofstream).

b. nodeTypeToString:
```cpp
static std::string nodeTypeToString(NodeType type) {
    switch (type) {
        case TOKEN_NODE:
            return "TOKEN_NODE";
        case PROGRAM:
        // DAN SETERUSNYA
    }
}
```
nodeTypeToString: Fungsi pemetaan utilitas eksternal yang mengonversi kode enum (integer) kembali menjadi format string (misal: enum 1 menjadi teks "PROGRAM") agar dapat dibaca oleh manusia saat Parse Tree dicetak.

### 2. File parsingTree.hpp 

Kelas ParsingTree merupakan inti dari modul Syntax Analysis. Arsitektur kelas ini mengimplementasikan algoritma Top-Down Recursive Descent Parsing secara deterministik untuk memvalidasi urutan pita token dari Lexer dan mengonversinya menjadi Parse Tree 2D.

#### State Management & Alokasi Memori:
```c++
private:
    Lexer& lexer;
    Token currentToken;
    Token nextToken;
    std::unique_ptr<Node> root;
```

Bagian ini menyimpan state operasional kompilator selama fase sintaksis berjalan. Objek lexer diikat menggunakan referensi (&) untuk memastikan parser membaca dari mesin DFA yang sama tanpa menyalin data. Variabel currentToken dan nextToken bertindak sebagai buffer yang menyimpan token yang sedang dievaluasi. Kepemilikan struktur pohon berakar pada root yang menggunakan smart pointer (std::unique_ptr); arsitektur ini menjamin pembersihan memori (garbage collection) pada seluruh cabang di bawahnya berjalan otomatis saat kompilasi selesai atau terhenti akibat crash, mengeliminasi risiko memory leak.

#### Core Parsing Utilities (Utilitas Validasi Dasar):
```cpp
void advance();
std::string tokenLabel(const Token& token) const;
std::unique_ptr<Node> accept(TokenType expectedType);
std::unique_ptr<Node> expect(TokenType expectedType);
```
Sekumpulan operasi primitif untuk memanipulasi pita token secara linier.

Metode advance() memicu Lexer untuk menimpa buffer dengan token valid berikutnya, secara otomatis mengabaikan spasi dan blok komentar.

accept() melakukan validasi non-kritis; jika token cocok, pointer maju, jika tidak, operasi diabaikan tanpa interupsi.

expect() adalah utilitas validasi absolut. Metode ini mewajibkan kecocokan token dengan spesifikasi tata bahasa. Kegagalan validasi pada titik ini merupakan pemicu mutlak (throw runtime_error) yang menghasilkan Syntax Error dan menghentikan kompilasi.

#### Predictive Lookahead (Himpunan FIRST)
```cpp
bool isStatementStart(TokenType type) const;
bool isFactorStart(TokenType type) const;
bool isRelationalOperator(TokenType type) const;
bool isAdditiveOperator(TokenType type) const;
bool isMultiplicativeOperator(TokenType type) const;
```
Fungsi-fungsi prediktif ini merupakan representasi kode dari himpunan FIRST dalam teori Context-Free Grammar (CFG). Parser menggunakan fungsi boolean ini sebagai mekanisme lookahead evaluasi untuk menguji validitas jenis token sebelum mengeksekusi percabangan instruksi (if-else atau while). Implementasi ini memastikan parser tidak masuk ke dalam fungsi rekursif yang salah saat menemui ambiguitas operasional, menjaga sifat deterministik dari algoritma.

#### Recursive Descent Implementations (Aturan Produksi)
```cpp
std::unique_ptr<Node> parseProgram();
std::unique_ptr<Node> parseDeclarationPart();
std::unique_ptr<Node> parseStatement();
std::unique_ptr<Node> parseExpression();
std::unique_ptr<Node> parseFactor();
```

Blok ini memuat sekumpulan metode rekursif internal yang dipetakan secara eksak (1:1) terhadap aturan tata bahasa spesifikasi. Ini adalah inti dari konstruksi hierarki; setiap fungsi bertanggung jawab mengikat daun (token node) menjadi cabang memori, saling memanggil melalui Call Stack C++ dari tingkat makro struktur program (parseProgram), turun ke blok deklarasi dan statement, hingga level paling mikro berupa evaluasi presedensi operator matematika tunggal (parseFactor).

#### Public Application Programming Interface (API):
```cpp
public:
    ParsingTree(Lexer& lexInstance);
    ~ParsingTree() = default;

    void build();
    void printToCLI();
    void exportToFile(const std::string& filename);
```
Antarmuka publik ini bertindak sebagai titik interaksi antara modul sintaksis dengan titik masuk utama (main.cpp).

Konstruktor diinjeksikan secara Dependency Injection dengan instansi Lexer.

Metode build() bertindak sebagai pemicu (trigger) utama untuk merangkai objek pohon mulai dari node puncak.

Setelah kompilasi sintaksis berhasil, printToCLI() dan exportToFile() disediakan untuk melintasi graf memori secara Depth-First Search (DFS) guna merender struktur hierarki 2D secara visual ke layar terminal atau menyimpannya sebagai berkas rekam jejak (log).

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
# Untuk Melakukan Keduanya
make clean && make run
```
Jika tidak menggunakan Makefile:
```bash
g++ -std=c++17 -Iinclude src/lexer.cpp src/main.cpp -o bin/compiler
```

### Contoh Input & Output:
#### Input:
```
program Hello;

var
  a, b, i: integer;

function AddTen(x: integer): integer;
begin
  AddTen := x + 10;
end;

begin
  a := 5;
  b := AddTen(a);
  writeln('Result = ', b);
end.

```
#### Output(Lexer):
```
programsy
ident(Hello)
semicolon
varsy
ident(a)
comma
ident(b)
comma
ident(i)
colon
ident(integer)
semicolon
functionsy
ident(AddTen)
lparent
ident(x)
colon
ident(integer)
rparent
colon
ident(integer)
semicolon
beginsy
ident(AddTen)
becomes
ident(x)
plus
intcon(10)
semicolon
endsy
semicolon
beginsy
ident(a)
becomes
intcon(5)
semicolon
ident(b)
becomes
ident(AddTen)
lparent
ident(a)
rparent
semicolon
ident(writeln)
lparent
string('Result = ')
comma
ident(b)
rparent
semicolon
endsy
period

```
#### Output(Parser):
```
<program>
├── <program-header>
│   ├── programsy
│   ├── ident(Hello)
│   └── semicolon
├── <declaration-part>
│   ├── <var-declaration>
│   │   ├── varsy
│   │   ├── <identifier-list>
│   │   │   ├── ident(a)
│   │   │   ├── comma
│   │   │   ├── ident(b)
│   │   │   ├── comma
│   │   │   └── ident(i)
│   │   ├── colon
│   │   ├── <type>
│   │   │   └── ident(integer)
│   │   └── semicolon
│   └── <subprogram-declaration>
│       └── <function-declaration>
│           ├── functionsy
│           ├── ident(AddTen)
│           ├── <formal-parameter-list>
│           │   ├── lparent
│           │   ├── <parameter-group>
│           │   │   ├── <identifier-list>
│           │   │   │   └── ident(x)
│           │   │   ├── colon
│           │   │   └── ident(integer)
│           │   └── rparent
│           ├── colon
│           ├── ident(integer)
│           ├── semicolon
│           ├── <block>
│           │   ├── <declaration-part>
│           │   └── <compound-statement>
│           │       ├── beginsy
│           │       ├── <statement-list>
│           │       │   ├── <statement>
│           │       │   │   └── <assignment-statement>
│           │       │   │       ├── <variable>
│           │       │   │       │   └── ident(AddTen)
│           │       │   │       ├── becomes
│           │       │   │       └── <expression>
│           │       │   │           └── <simple-expression>
│           │       │   │               ├── <term>
│           │       │   │               │   └── <factor>
│           │       │   │               │       └── <procedure/function-call>
│           │       │   │               │           └── ident(x)
│           │       │   │               ├── <additive-operator>
│           │       │   │               │   └── plus
│           │       │   │               └── <term>
│           │       │   │                   └── <factor>
│           │       │   │                       └── intcon(10)
│           │       │   └── semicolon
│           │       └── endsy
│           └── semicolon
├── <compound-statement>
│   ├── beginsy
│   ├── <statement-list>
│   │   ├── <statement>
│   │   │   └── <assignment-statement>
│   │   │       ├── <variable>
│   │   │       │   └── ident(a)
│   │   │       ├── becomes
│   │   │       └── <expression>
│   │   │           └── <simple-expression>
│   │   │               └── <term>
│   │   │                   └── <factor>
│   │   │                       └── intcon(5)
│   │   ├── semicolon
│   │   ├── <statement>
│   │   │   └── <assignment-statement>
│   │   │       ├── <variable>
│   │   │       │   └── ident(b)
│   │   │       ├── becomes
│   │   │       └── <expression>
│   │   │           └── <simple-expression>
│   │   │               └── <term>
│   │   │                   └── <factor>
│   │   │                       └── <procedure/function-call>
│   │   │                           ├── ident(AddTen)
│   │   │                           ├── lparent
│   │   │                           ├── <parameter-list>
│   │   │                           │   └── <expression>
│   │   │                           │       └── <simple-expression>
│   │   │                           │           └── <term>
│   │   │                           │               └── <factor>
│   │   │                           │                   └── <procedure/function-call>
│   │   │                           │                       └── ident(a)
│   │   │                           └── rparent
│   │   ├── semicolon
│   │   ├── <statement>
│   │   │   └── <procedure/function-call>
│   │   │       ├── ident(writeln)
│   │   │       ├── lparent
│   │   │       ├── <parameter-list>
│   │   │       │   ├── <expression>
│   │   │       │   │   └── <simple-expression>
│   │   │       │   │       └── <term>
│   │   │       │   │           └── <factor>
│   │   │       │   │               └── string('Result = ')
│   │   │       │   ├── comma
│   │   │       │   └── <expression>
│   │   │       │       └── <simple-expression>
│   │   │       │           └── <term>
│   │   │       │               └── <factor>
│   │   │       │                   └── <procedure/function-call>
│   │   │       │                       └── ident(b)
│   │   │       └── rparent
│   │   └── semicolon
│   └── endsy
└── period

```

### Pembagian Tugas:
#### ***[Milestone 1]:***
| NIM | Nama | Pembagian Tugas |Persentase Kontribusi|
| :---: | :---: | :---: |:---:
| 13524114 | Mirza Tsabita Wafa'ana | Membuat Laporan dan membuat diagram transisi DFA |25  %
| 13524119 | Nathanael Shane Bennet | Membuat source code program dan Mengerjakan Laporan|25 %
|13524130|Faris Wirakusuma Triawan|Membuat source code program dan Mengerjakan Laporan|25 %
|13524144|Jonathan Harijadi|Mengerjakan Laporan dan membuat diagram transisi DFA|25%


#### ***[Milestone 2]:***

| NIM | Nama | Pembagian Tugas |Persentase Kontribusi|
| :---: | :---: | :---: |:---:
| 13524114 | Mirza Tsabita Wafa'ana | Pengaplikasian node for-statement hingga node multiplicative-operator dan mengerjakan laporan |25  %
| 13524119 | Nathanael Shane Bennet | Pengaplikasian node statement-list hingga node repeat-statement dan mengerjakan laporan|25 %
|13524130|Faris Wirakusuma Triawan|Pengaplikasian node program hingga node range,inisialisasi syntax analizer dan mengerjakan laporan|25 %
|13524144|Jonathan Harijadi|Pengaplikasian node enumerated hingga node compound-statement dan mengerjakan laporan|25%
