#include "AST_Tree.hpp"
#include <stdexcept>


unique_ptr<ASTNode> AST_Tree::build(Node* cstRoot) {
    if (!cstRoot) return nullptr;

    // 1. Kontainer Utama
    // Tambahkan DECLARATION_PART agar deklarasi tidak terlewat
    if (cstRoot->type == COMPOUND_STATEMENT || cstRoot->type == STATEMENT_LIST || 
        cstRoot->type == PROGRAM || cstRoot->type == BLOCK || cstRoot->type == CASE_BLOCK ||
        cstRoot->type == DECLARATION_PART) {
        
        auto block = make_unique<BlockNode>();
        for (auto& child : cstRoot->children) {
            auto result = build(child.get());
            if (result) {
                block->statements.push_back(std::move(result));
            }
        }
        return block->statements.empty() ? nullptr : std::move(block);
    }

    // 2. Navigasi Statement (Pass-through)
    if (cstRoot->type == STATEMENT) {
        if (!cstRoot->children.empty()) return build(cstRoot->children[0].get());
    }

    // 3. Mapping Node Eksekusi & Control Flow
    if (cstRoot->type == ASSIGNMENT_STATEMENT) return buildAssignment(cstRoot);
    if (cstRoot->type == PROCEDURE_FUNCTION_CALL) return buildFunctionCall(cstRoot);
    if (cstRoot->type == IF_STATEMENT) return buildIfStatement(cstRoot);
    if (cstRoot->type == WHILE_STATEMENT) return buildWhileStatement(cstRoot);
    if (cstRoot->type == REPEAT_STATEMENT) return buildRepeatStatement(cstRoot);
    if (cstRoot->type == FOR_STATEMENT) return buildForStatement(cstRoot);
    if (cstRoot->type == CASE_STATEMENT) return buildCaseStatement(cstRoot);

    // 4. Mapping Node Deklarasi
    if (cstRoot->type == VAR_DECLARATION) return buildVarDeclaration(cstRoot);
    if (cstRoot->type == FUNCTION_DECLARATION || cstRoot->type == PROCEDURE_DECLARATION) return buildFunctionDeclaration(cstRoot);

    // 5. Mapping Akses Variabel Kompleks (Array / Record)
    if (cstRoot->type == COMPONENT_VARIABLE) return buildComponentVariable(cstRoot);

    // 6. Safety Net
    for (auto& child : cstRoot->children) {
        auto res = build(child.get());
        if (res) return res; 
    }

    return nullptr;
}

unique_ptr<ASTNode> AST_Tree::buildComponentVariable(Node* cstNode) {
    // Karena ini merepresentasikan p1.x atau numbers[i], kita ekstrak string lengkapnya
    // extractVarName yang rekursif akan menelusuri ke bawah dan mengambil identitas tokennya.
    
    // Jika CST lu memisahkan record name dan field name di children[0] dan children[2] untuk record:
    if (cstNode->children.size() >= 3 && cstNode->children[1]->lexeme == ".") {
        string recordName = extractVarName(cstNode->children[0].get());
        string fieldName = extractVarName(cstNode->children[2].get());
        return make_unique<FieldAccessNode>(recordName, fieldName);
    }
    
    // Fallback: gabungkan nama variabel jika ini sekadar array index atau tidak masuk format record di atas
    string varName = extractVarName(cstNode);
    return make_unique<VariableNode>(varName);
}

unique_ptr<ASTNode> AST_Tree::buildAssignment(Node* cstNode) {
    string varName = extractVarName(cstNode->children[0].get());
    Node* exprNode = nullptr;
    for (auto& child : cstNode->children) {
        if (child->type == EXPRESSION) {
            exprNode = child.get();
            break;
        }
    }
    return make_unique<AssignNode>(varName, buildExpression(exprNode));
}

unique_ptr<ASTNode> AST_Tree::buildFunctionCall(Node* cstNode) {
    string funcName = extractVarName(cstNode->children[0].get());
    vector<unique_ptr<ASTNode>> args;

    for (auto& child : cstNode->children) {
        if (child->type == PARAMETER_LIST) {
            for (auto& pChild : child->children) {
                if (pChild->type == EXPRESSION) {
                    args.push_back(buildExpression(pChild.get()));
                }
            }
        }
    }
    return make_unique<CallNode>(funcName, move(args));
}
unique_ptr<ASTNode> AST_Tree::buildIfStatement(Node* cstNode) {
    
    unique_ptr<ASTNode> condition = buildExpression(cstNode->children[1].get());
    unique_ptr<ASTNode> thenBranch = build(cstNode->children[3].get());
    unique_ptr<ASTNode> elseBranch = nullptr;

    // Cek keberadaan ELSE (index 4 adalah 'elsesy')
    if (cstNode->children.size() > 5) {
        elseBranch = build(cstNode->children[5].get());
    }

    return make_unique<IfNode>(move(condition), move(thenBranch), move(elseBranch));
}
unique_ptr<ASTNode> AST_Tree::buildExpression(Node* cstNode) {
    if (!cstNode) return nullptr;

    if (cstNode->type == EXPRESSION || cstNode->type == SIMPLE_EXPRESSION || cstNode->type == TERM) {
        if (cstNode->children.size() == 1) {
            return buildExpression(cstNode->children[0].get());
        }
        string op = cstNode->children[1]->lexeme;
        return make_unique<BinOpNode>(op, buildExpression(cstNode->children[0].get()), buildExpression(cstNode->children[2].get()));
    }

    if (cstNode->type == FACTOR) {
        Node* primary = cstNode->children[0].get();
        if (primary->lexeme == "(") {
            return buildExpression(cstNode->children[1].get());
        }
        if (primary->type == TOKEN_NODE || primary->type == VARIABLE) {
            string rawVal = extractVarName(primary);
            if (primary->lexeme.find("string_tok") == 0 || (!rawVal.empty() && rawVal[0] == '\'')) {
                return make_unique<StringNode>(rawVal);
            }
            if (primary->lexeme.find("intcon") == 0 || isdigit(rawVal[0])) {
                return make_unique<NumberNode>(rawVal);
            }
            return make_unique<VariableNode>(rawVal);
        }
        return buildExpression(primary);
    }
    return nullptr;
}
// 1. Ganti extractVarName dengan implementasi ini
string AST_Tree::extractVarName(Node* cstNode) {
    if (!cstNode) return "";

    // Basis rekursi: Jika ini TOKEN_NODE, ini tempat nama disimpan
    if (cstNode->type == TOKEN_NODE) {
        string lex = cstNode->lexeme;
        size_t start = lex.find('(');
        size_t end = lex.rfind(')');
        if (start != string::npos && end != string::npos && start < end) {
            return lex.substr(start + 1, end - start - 1);
        }
        return lex; // Fallback jika tidak ada format ident(...)
    }

    // Jika bukan TOKEN_NODE, terus turun ke anak pertama sampai ketemu
    if (!cstNode->children.empty()) {
        return extractVarName(cstNode->children[0].get());
    }

    return "";
}
unique_ptr<ASTNode> AST_Tree::buildWhileStatement(Node* cstNode) {
    // WHILE_STATEMENT: [1]Expression, [3]CompoundStatement
    auto cond = buildExpression(cstNode->children[1].get());
    auto body = build(cstNode->children[3].get());
    return make_unique<WhileNode>(move(cond), move(body));
}

unique_ptr<ASTNode> AST_Tree::buildRepeatStatement(Node* cstNode) {
    // REPEAT_STATEMENT: [1]StatementList, [3]Expression
    auto body = build(cstNode->children[1].get());
    auto cond = buildExpression(cstNode->children[3].get());
    return make_unique<RepeatNode>(move(body), move(cond));
}

unique_ptr<ASTNode> AST_Tree::buildForStatement(Node* cstNode) {
    // FOR_STATEMENT: [1]ident, [3]expr(start), [5]expr(end), [7]CompoundStatement
    string varName = extractVarName(cstNode->children[1].get());
    auto start = buildExpression(cstNode->children[3].get());
    auto end = buildExpression(cstNode->children[5].get());
    auto body = build(cstNode->children[7].get());
    return make_unique<ForNode>(varName, move(start), move(end), move(body));
}

unique_ptr<ASTNode> AST_Tree::buildCaseStatement(Node* cstNode) {
    // CASE_STATEMENT: [1]Expression, [3]CaseBlock
    auto expr = buildExpression(cstNode->children[1].get());
    auto caseNode = make_unique<CaseNode>(move(expr));
    
    // Proses CaseBlock
    Node* block = cstNode->children[3].get();
    for (auto& child : block->children) {
        if (child->type == CASE_BLOCK) {
            // Asumsi: Constant = child->children[0], Statement = child->children[2]
            string constVal = extractVarName(child->children[0].get());
            caseNode->branches.push_back({constVal, build(child->children[2].get())});
        }
    }
    return caseNode;
}


unique_ptr<ASTNode> AST_Tree::buildVarDeclaration(Node* cstNode) {
    auto node = make_unique<VarDeclNode>();
    // Asumsi: CST VAR_DECLARATION = [varsy, IDENTIFIER_LIST, colon, TYPE, semicolon]
    if (cstNode->children.size() >= 4) {
        Node* idList = cstNode->children[1].get();
        for (auto& child : idList->children) {
            if (child->type == TOKEN_NODE) node->varNames.push_back(extractVarName(child.get()));
        }
        node->type = extractVarName(cstNode->children[3].get());
    }
    return node;
}

unique_ptr<ASTNode> AST_Tree::buildCompoundStatement(Node* cstNode) {
    // Struktur: [beginsy, STATEMENT_LIST, endsy]
    if (cstNode->children.size() >= 2) {
        return build(cstNode->children[1].get());
    }
    return nullptr;
}

unique_ptr<ASTNode> AST_Tree::buildFunctionDeclaration(Node* cstNode) {
    // Asumsi: [functionsy, ident(name), FORMAL_PARAMS, colon, TYPE, semicolon, BLOCK]
    string name = extractVarName(cstNode->children[1].get());
    unique_ptr<ASTNode> block = build(cstNode->children.back().get());
    return make_unique<SubprogramDeclNode>(name, move(block));
}
unique_ptr<ASTNode> AST_Tree::buildProcedureDeclaration(Node* cstNode) {
    if (!cstNode || cstNode->children.empty()) return nullptr;

    // Ekstraksi nama prosedur. Biasanya berada di indeks ke-1 setelah keyword 'procedure'
    string name = extractVarName(cstNode->children[1].get());

    // Cari node yang merepresentasikan blok eksekusi prosedur
    Node* blockNode = nullptr;
    for (auto& child : cstNode->children) {
        if (child->type == BLOCK || child->type == COMPOUND_STATEMENT || child->type == DECLARATION_PART) {
            blockNode = child.get();
            break;
        }
    }

    // Jika tipe spesifik tidak ditemukan, gunakan anak terakhir sebagai fallback
    if (!blockNode && !cstNode->children.empty()) {
        blockNode = cstNode->children.back().get();
    }

    return make_unique<SubprogramDeclNode>(name, build(blockNode));
}
unique_ptr<ASTNode> AST_Tree::buildFieldAccess(Node* cstNode) {
    // Menangani p1.x
    // Ambil record (p1) dan field (x)
    string recordName = extractVarName(cstNode->children[0].get());
    string fieldName = extractVarName(cstNode->children[2].get());
    return make_unique<FieldAccessNode>(recordName, fieldName);
}

unique_ptr<ASTNode> AST_Tree::buildEmptyStatement(Node* cstNode) {
    return nullptr; // Tidak melakukan aksi apa-apa
}