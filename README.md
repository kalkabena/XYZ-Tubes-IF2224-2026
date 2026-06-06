# ⚙️ ARION COMPILER

![Version](https://img.shields.io/badge/Release-v0.4.1-blue)
![Build](https://img.shields.io/badge/Build-Makefile_Ready-success)
![Language](https://img.shields.io/badge/Language-C++17-orange)

**Nama Kelompok:** Empty String

### 👥 Anggota Kelompok
| NIM | Nama |
| :---: | :--- |
| 13524114 | Mirza Tsabita Wafa'ana |
| 13524119 | Nathanael Shane Bennet |
| 13524130 | Faris Wirakusuma Triawan |
| 13524144 | Jonathan Harijadi |

---

## 📝 Deskripsi Program
Program kompilator Arion dibuat menggunakan bahasa **C++**. Arsitektur ini dibangun agar **Makefile Ready** sehingga proses *build*, kompilasi, dan *testing* dapat dilakukan secara terotomatisasi dengan satu perintah. Implementasi arsitektur kompilator ini dibagi menjadi beberapa modul utama (*Milestones*) yang mencakup seluruh *pipeline* kompilasi, mulai dari analisis leksikal, analisis sintaksis, pembentukan *Decorated AST*, generasi *Intermediate Code*, hingga eksekusi instruksi melalui *Virtual Machine*.

## 🔍 A. LEXICAL ANALYZER (Milestone 1)

### 1. File `token.hpp`
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

###  ***C. SEMANTIC ANALYZER & SYMBOL TABLE <Milestone_3>***

Milestone 3 berfokus pada tahapan analisis semantik awalan. Pada tahap ini, dilakukan proses penyederhanaan *Concrete Syntax Tree* (CST) menjadi *Abstract Syntax Tree* (AST), serta pembuatan *Symbol Table* untuk melacak dan menyimpan informasi terkait identifier program.

*(Catatan: Implementasi Error Handling untuk mendeteksi semantic error seperti ketidaksesuaian tipe data, pemanggilan fungsi tak terdefinisi, atau penggunaan variabel yang belum dideklarasikan saat ini sedang dalam pengerjaan dan akan ditambahkan pada *update* berikutnya).*

### 1. Konversi CST ke AST (ASTNode.hpp & AST_Tree.cpp)
*Concrete Syntax Tree* (CST) yang dihasilkan pada Milestone 2 memuat sangat banyak node struktural dari tata bahasa sintaksis (seperti `STATEMENT_LIST`, `SIMPLE_EXPRESSION`, serta token delimiter/kurung pembantu) yang tidak lagi relevan untuk tahap pengerjaan (eksekusi/evaluasi). Konversi CST ke AST bertujuan untuk membuang percabangan redundan tersebut sehingga menyisakan struktur logika kode yang padat.

Berikut adalah cuplikan ilustrasi dari abstraksi node dalam struktur AST:
```cpp
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(std::ostream& os, std::string prefix = "", bool isLast = true) const = 0;
};

// Contoh turunan node operasional yang padat
class AssignNode : public ASTNode {
    std::unique_ptr<ASTNode> variable;
    std::unique_ptr<ASTNode> expression;
};
```

- **ASTNode.hpp**: Kelas ini mendefinisikan abstraksi dasar dari pohon AST beserta node-node turunannya yang fungsional (contoh: `AssignNode`, `CallNode`, `BinOpNode`, `IfNode`, `WhileNode`, `ForNode`, dsb).
- **AST_Tree.cpp**: Berfungsi menelusuri objek CST secara rekursif. Konverter membedah blok node makro ke bentuk yang paling mikro guna mengekstrak *identifier* maupun perhitungannya secara utuh, lalu menyusunnya menjadi instance objek kelas *Abstract Syntax Tree*.

### 2. Symbol Table (SymbolTable.hpp & SymbolTable.cpp)
*Symbol Table* dirancang khusus untuk memetakan nama *identifier* (konstanta, tipe, variabel, prosedur, fungsi) yang muncul di program dengan properti detail yang melekat padanya. Menggunakan struktur array dinamis yang mensimulasikan pendekatan *block-structured*:

Berikut adalah implementasi struktur dasar elemen tabel (*TabEntry*) dari Main Symbol Table:
```cpp
struct TabEntry {
    std::string name; 
    int link;        
    ObjectClass obj; // Variabel, fungsi, prosedur, tipe, konstanta
    DataType type;   // Tipe data: Integer, Real, Array, dsb. 
    int ref;         // Menunjuk ke tabel referensi lain (seperti ATAB/BTAB)
    int nrm;         
    int lev;         // Level kedalaman Scope
    int adr;         
};
```
Terdapat tiga tabel inti yang saling merelasikan informasi identifier:
- **TAB (Main Symbol Table)**: Menyimpan semua entri *identifier* dengan informasi vital meliputi nama, kategori objek (`OBJ_VARIABLE`, `OBJ_PROCEDURE`, dll), tipe objek (`TYPE_INTEGER`, `TYPE_REAL`, dll), tingkat ruang lingkup/scope eksekusi (`lev`), dan referensinya.
- **ATAB (Array Table)**: Tabel pembantu khusus dalam mendefinisikan Array/Struktur jamak. Tabel ini mencatat tipe indeks (`xtyp`), tipe elemen dasar (`etyp`), batas bawah alokasi (`low`), batas atas (`high`), dan total memori/elemen komputasinya.
- **BTAB (Block Table)**: Tabel referensi dari ruang lingkup (*scope*) dari berbagai prosedur maupun blok utama. Mencatat relasi cakupan yang digunakan untuk mengetahui jumlah argumen fungsi/prosedur (`lpar`) dan total ukuran alokasi variabel memori di block tersebut (`vsze`).

Untuk mempopulasikan *Symbol Table*, digunakan metode `traverseNode` yang menelusuri CST mulai dari root Node (*Depth-First Search*) seperti berikut:
```cpp
void SymbolTable::buildFromNode(Node* cstRoot) {
    currentLev = 0;
    currentBlock = 0;
    
    // Inisialisasi blok global awal program (Level 0)
    btab.last = 0; 
    btab.lpar = 0;
    btab.psze = 0;
    btab.vsze = 0;

    traverseNode(cstRoot, currentLev);
}
```
Fungsi `traverseNode` akan mendeteksi setiap pembuatan properti program dan secara otomatis mengekstraksi nama serta tipe dan mendaftarkannya ke dalam tabel dinamis terkait (TAB/ATAB/BTAB).

###  ***D. INTERMEDIATE CODE GENERATOR & INTERPRETER <Milestone_4>***

Milestone 4 berfokus pada tahapan pembuatan *Intermediate Code Generator* (ICG) yang menerima input berupa *Decorated Abstract Syntax Tree* (AST) dari analisis semantik, serta eksekusinya melalui *Interpreter* berbasis mesin tumpukan yang aman (*Safe Stack Machine*).

### 1. Intermediate Code Generator (`ICGenerator.hpp` & `ICGGenerator.cpp`)
Tahap pembangkitan kode antara (*Intermediate Code Generation*) diimplementasikan melalui penelusuran (*traversal*) pada *Decorated AST*. Arsitektur ICG secara komprehensif memetakan setiap *node* AST menjadi sekumpulan instruksi *Three-Address Code* (TAC) dan operasi *Stack Machine* tingkat rendah yang siap dieksekusi oleh mesin virtual.

Berikut adalah cuplikan implementasi emisi instruksi pada fungsi inti ICG:
```cpp
int ICGenerator::emit(const string& op, int level, int arg) {
    code.push_back({currentLine, op, level, arg});
    return currentLine++;
}

void ICGenerator::generate(ASTNode* node) {
    emit("INT", 0, 1000);   // Inisialisasi alokasi stack global
    if (node) node->accept(this);
    emit("RET", 0, 0);      // Return (akhiri eksekusi utama)
}
```

- **`src/icg/ICGGenerator.cpp`**: Bertindak sebagai mesin inti yang menavigasi setiap struktur bahasa, mulai dari deklarasi variabel hingga struktur kontrol program (*if, while, for*), dan membangkitkan set instruksi (*opcode*) seperti `LIT`, `LOD`, `STO`, `JMP`, `OPR`, serta operasi aritmatika.
- **`include/icg/TACInstruction.hpp`**: Menyediakan definisi struktur data (*struct*) instruksi TAC beserta instrumen *formatting* agar kode antara yang dihasilkan dapat divisualisasikan dengan terstruktur.

### 2. Interpreter & Virtual Machine (`VirtualMachine.cpp`, `SafeStackMachine.cpp`, dll.)
*Interpreter* bertanggung jawab penuh untuk memproses instruksi *opcode* yang dibangkitkan oleh ICG. Lingkungan eksekusi ini disimulasikan menggunakan paradigma *Safe Stack Machine* yang menerapkan kontrol memori ketat dan proteksi *runtime*.

Berikut adalah cuplikan siklus *Fetch-Decode-Execute* pada mesin virtual:
```cpp
void VirtualMachine::run() {
    instructionPointer = 0;
    memory.reset();

    while (instructionPointer < static_cast<int>(code.size())) {
        TACInstruction instr = code[instructionPointer];
        instructionPointer++; // Pindahkan penunjuk instruksi (PC)

        if (instr.op == "LIT") {
            memory.push(instr.arg); // Dorong nilai literal ke stack
        }
        else if (instr.op == "LOD") {
            memory.push(memory.getValueAt(instr.arg)); // Muat variabel
        }
        else if (instr.op == "STO") {
            int val = memory.pop();
            memory.setValueAt(instr.arg, val); // Simpan hasil ke memori
        }
        // Dan eksekusi instruksi kontrol (JMP, OPR) lainnya...
    }
}
```

- **`src/interpreter/VirtualMachine.cpp`** & **`VirtualMachine.hpp`**: Merupakan abstraksi siklus *Fetch-Decode-Execute* (*Virtual Machine*). *VM* membaca urutan instruksi melalui manipulasi *Program Counter* (*PC*) dan mendelegasikan eksekusinya ke lapisan *Stack Machine*.
- **`src/interpreter/SafeStackMachine.cpp`** & **`SafeStackMachine.hpp`**: Bertugas mengeksekusi langsung set instruksi arsitektur internal program (*ISA*). Modul ini bekerja bersama **`src/interpreter/StackMemory.cpp`** untuk mengelola alokasi ruang memori global dan operasional per blok *stack*.
- **`src/interpreter/RuntimeProtection.cpp`** & **`RuntimeProtection.hpp`**: Menyediakan sub-modul keamanan eksekusi (*Runtime Protection*). Sub-modul ini memastikan program kebal terhadap kesalahan memori (*Memory Out-of-Bounds*), pembagian dengan nol (*Division by Zero*), kegagalan operasi tumpukan (*Stack Underflow*), maupun *overflow* numerik. Modul ini secara konstan dipantau kesehatannya oleh prosedur validasi internal dalam **`src/interpreter/RuntimeProtectionSelfTest.cpp`**.

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

#### Panduan Menjalankan Program (Alur Kompilasi & Input/Output):

**1. Kompilasi & Menjalankan Program (Compile & Run):**
Gunakan perintah `make` untuk melakukan proses *build* sekaligus langsung mengeksekusi kompilator:
```bash
make run
```
Atau jika Anda hanya ingin menjalankan *executable* yang sudah di-*build* sebelumnya secara manual:
```bash
./bin/compiler
```

**2. Memasukkan File Input (Input):**
Saat program berjalan, Anda akan disambut dengan *prompt* interaktif di CLI seperti berikut:
`Enter input file path (or 'q' to quit):`

Silakan ketikkan *path* (rute) menuju file yang ingin diuji (file bahasa Arion/Pascal berekstensi `.txt`) yang sudah Anda siapkan. Misalnya:
`test/input/testcase1.txt`
Lalu tekan **Enter**.

**3. Melihat Hasil Output:**
Setelah proses kompilasi sukses dieksekusi, hasil dari tahapan Lexer, Parser (Parse Tree), dan Analisis Semantik (Symbol Table & AST) akan **langsung ditampilkan di layar CLI Anda**.

Selain dicetak di layar, program secara otomatis mengekspor/menyimpan salinan *log* output tersebut ke dalam folder `test/output/`. Anda dapat memeriksa hasil lengkapnya pada:
- `test/output/milestone_1.txt` *(Hasil Token Lexer)*
- `test/output/milestone_2.txt` *(Hasil Parse Tree Sintaksis)*
- `test/output/milestone_3.txt` *(Hasil Analisis Semantik: Symbol Table & AST)*
- `test/output/icg_intepreter_report.txt` *(Hasil Intermediate Code & Output Interpreter)*
- `test/output/vulnerability_report.txt` *(Hasil Runtime Protection Self Test)*

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

#### Output(Symbol Table & AST - Milestone 3):
```
--- SYMBOL TABLE (TAB) ---
idx  id             obj         type     ref  nrm  lev  adr  link
-----------------------------------------------------------------
33   Hello          4           0       0    1    0    0    0
34   a              2           1       0    1    0    0    0
35   b              2           1       0    1    0    0    0
36   i              2           1       0    1    0    0    0
37   AddTen         5           1       0    1    0    0    0
38   x              2           1       0    1    1    0    0

--- ARRAY TABLE (ATAB) ---
idx  xtyp   etyp   eref   low    high   elsz   size
---------------------------------------------------

--- BLOCK TABLE (BTAB) ---
idx  last  lpar  psze  vsze
---------------------------
0    0     0     0     0
1    37    0     0     3
2    38    1     0     1

--- DECORATED AST ---
└── BlockNode
    ├── BlockNode
    │   ├── VarDeclNode(Type: integer)
    │   │   ├── a
    │   │   ├── comma
    │   │   ├── b
    │   │   ├── comma
    │   │   └── i
    │   └── SubprogramDecl(AddTen)
    │       └── [NULL BLOCK]
    └── BlockNode
        └── BlockNode
            ├── AssignNode
            │   ├── VariableNode(a)
            │   └── NumberNode(5)
            ├── AssignNode
            │   ├── VariableNode(b)
            │   └── CallNode(func: 'AddTen')
            │       └── VariableNode(a)
            └── CallNode(func: 'writeln')
                ├── StringNode('Result = ')
                └── VariableNode(b)

```

#### Output(ICG & Interpreter - Milestone 4):
```
--- Intermediate Code Generation Result ---

0 INT 0 1000
1 JMP 0 8
2 INT 0 15
3 LOD 0 0
4 LIT 0 10
5 OPR 0 2
6 STO 0 0
7 RET 0 0
8 LIT 0 5
9 STO 0 5
10 LIT 0 0
11 STO 0 6
12 LOD 0 6
13 OPR 0 14
14 RET 0 0

--- Runtime Protection Self Test ---

Test: TC-01 Stack underflow pada OPR ADD - PASSED
Test: TC-02 Invalid jump target - PASSED
Test: TC-03 Memory out-of-bounds pada LOD - PASSED
Test: TC-04 Numerical overflow pada ADD - PASSED
Test: TC-05 Division by zero - PASSED
Test: TC-06 Array index out-of-bounds helper - PASSED
Test: TC-07 Program valid: y := 10 + 5; writeln(y) - PASSED

--- Safe Stack Machine Execution ---

0
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

#### ***[Milestone 3]:***

| NIM | Nama | Pembagian Tugas | Persentase Kontribusi |
| :---: | :---: | :---: | :---: |
| 13524114 | Mirza Tsabita Wafa'ana | Mengerjakan bagian AST_Tree dan mengerjakan laporan | 25% |
| 13524119 | Nathanael Shane Bennet | Mengerjakan bagian symbol table dan mengerjakan laporan | 25% |
| 13524130 | Faris Wirakusuma Triawan | Mengerjakan bagian symbol table dan mengerjakan laporan | 25% |
| 13524144 | Jonathan Harijadi | Mengerjakan bagian AST_Tree dan mengerjakan laporan | 25% |

#### ***[Milestone 4]:***

| NIM | Nama | Pembagian Tugas | Persentase Kontribusi |
| :---: | :---: | :---: | :---: |
| 13524114 | Mirza Tsabita Wafa'ana | Mengerjakan kode bagian vulnerability testing dan mengerjakan laporan | 33% |
| 13524119 | Nathanael Shane Bennet | - | 0% |
| 13524130 | Faris Wirakusuma Triawan | Mengerjakan kode bagian ICG dan VirtualMachine dan mengerjakan laporan | 33% |
| 13524144 | Jonathan Harijadi | Mengerjakan kode bagian StackMachine dan mengerjakan laporan | 33% |
