#include "AST_Tree.hpp"
#include <stdexcept>


unique_ptr<ASTNode> AST_Tree::build(Node* cstRoot) {
    if (!cstRoot) return nullptr;
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

    if (cstRoot->type == STATEMENT) {
        if (!cstRoot->children.empty()) return build(cstRoot->children[0].get());
    }

    if (cstRoot->type == ASSIGNMENT_STATEMENT) return buildAssignment(cstRoot);
    if (cstRoot->type == PROCEDURE_FUNCTION_CALL) return buildFunctionCall(cstRoot);
    if (cstRoot->type == IF_STATEMENT) return buildIfStatement(cstRoot);
    if (cstRoot->type == WHILE_STATEMENT) return buildWhileStatement(cstRoot);
    if (cstRoot->type == REPEAT_STATEMENT) return buildRepeatStatement(cstRoot);
    if (cstRoot->type == FOR_STATEMENT) return buildForStatement(cstRoot);
    if (cstRoot->type == CASE_STATEMENT) return buildCaseStatement(cstRoot);

    if (cstRoot->type == VAR_DECLARATION) return buildVarDeclaration(cstRoot);
    if (cstRoot->type == FUNCTION_DECLARATION || cstRoot->type == PROCEDURE_DECLARATION) return buildFunctionDeclaration(cstRoot);

    if (cstRoot->type == COMPONENT_VARIABLE) return buildComponentVariable(cstRoot);

    // 6. Safety Net
    for (auto& child : cstRoot->children) {
        auto res = build(child.get());
        if (res) return res; 
    }

    return nullptr;
}

unique_ptr<ASTNode> AST_Tree::buildComponentVariable(Node* cstNode) {
    if (cstNode->children.size() >= 3 && cstNode->children[1]->lexeme == ".") {
        string recordName = extractVarName(cstNode->children[0].get());
        string fieldName = extractVarName(cstNode->children[2].get());
        return make_unique<FieldAccessNode>(recordName, fieldName);
    }
    string varName = extractVarName(cstNode);
    return make_unique<VariableNode>(varName);
}

unique_ptr<ASTNode> AST_Tree::buildAssignment(Node* cstNode) {
    // cstNode = ASSIGNMENT_STATEMENT
    Node* varNode = cstNode->children[0].get(); // Node VARIABLE
    Node* exprNode = cstNode->children[2].get(); // Node EXPRESSION

    unique_ptr<ASTNode> targetAST;

    // Cek apakah ada operasi array access / component variable di dalam VARIABLE
    if (varNode->children.size() > 1 && varNode->children[1]->type == COMPONENT_VARIABLE) {
        Node* compNode = varNode->children[1].get();
        std::string arrayName = extractVarName(varNode->children[0].get());
        
        if (!compNode->children.empty() && compNode->children[0]->lexeme.find("lbrack") != std::string::npos) {
            Node* indexListNode = compNode->children[1].get(); 
            auto indexExpr = build(indexListNode); 
            targetAST = make_unique<ArrayAccessNode>(arrayName, std::move(indexExpr));
        }
    } 
    if (!targetAST) {
        targetAST = make_unique<VariableNode>(extractVarName(varNode->children[0].get()));
    }

    return make_unique<AssignNode>(std::move(targetAST), buildExpression(exprNode));
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
string AST_Tree::extractVarName(Node* cstNode) {
    if (!cstNode) return "";

    if (cstNode->type == TOKEN_NODE) {
        string lex = cstNode->lexeme;
        size_t start = lex.find('(');
        size_t end = lex.rfind(')');
        if (start != string::npos && end != string::npos && start < end) {
            return lex.substr(start + 1, end - start - 1);
        }
        return lex;
    }

    if (!cstNode->children.empty()) {
        return extractVarName(cstNode->children[0].get());
    }

    return "";
}
unique_ptr<ASTNode> AST_Tree::buildWhileStatement(Node* cstNode) {
    auto cond = buildExpression(cstNode->children[1].get());
    auto body = build(cstNode->children[3].get());
    return make_unique<WhileNode>(move(cond), move(body));
}

unique_ptr<ASTNode> AST_Tree::buildRepeatStatement(Node* cstNode) {
    auto body = build(cstNode->children[1].get());
    auto cond = buildExpression(cstNode->children[3].get());
    return make_unique<RepeatNode>(move(body), move(cond));
}

unique_ptr<ASTNode> AST_Tree::buildForStatement(Node* cstNode) {
    string varName = extractVarName(cstNode->children[1].get());
    auto start = buildExpression(cstNode->children[3].get());
    auto end = buildExpression(cstNode->children[5].get());
    auto body = build(cstNode->children[7].get());
    return make_unique<ForNode>(varName, move(start), move(end), move(body));
}

unique_ptr<ASTNode> AST_Tree::buildCaseStatement(Node* cstNode) {
    auto expr = buildExpression(cstNode->children[1].get());
    auto caseNode = make_unique<CaseNode>(move(expr));
    Node* block = cstNode->children[3].get();
    for (auto& child : block->children) {
        if (child->type == CASE_BLOCK) {
            string constVal = extractVarName(child->children[0].get());
            caseNode->branches.push_back({constVal, build(child->children[2].get())});
        }
    }
    return caseNode;
}


unique_ptr<ASTNode> AST_Tree::buildVarDeclaration(Node* cstNode) {
    auto node = make_unique<VarDeclNode>();
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
    if (cstNode->children.size() >= 2) {
        return build(cstNode->children[1].get());
    }
    return nullptr;
}

unique_ptr<ASTNode> AST_Tree::buildFunctionDeclaration(Node* cstNode) {
    string name = extractVarName(cstNode->children[1].get());
    unique_ptr<ASTNode> block = build(cstNode->children.back().get());
    return make_unique<SubprogramDeclNode>(name, move(block));
}
unique_ptr<ASTNode> AST_Tree::buildProcedureDeclaration(Node* cstNode) {
    if (!cstNode || cstNode->children.empty()) return nullptr;

    string name = extractVarName(cstNode->children[1].get());
    Node* blockNode = nullptr;
    for (auto& child : cstNode->children) {
        if (child->type == BLOCK || child->type == COMPOUND_STATEMENT || child->type == DECLARATION_PART) {
            blockNode = child.get();
            break;
        }
    }

    if (!blockNode && !cstNode->children.empty()) {
        blockNode = cstNode->children.back().get();
    }

    return make_unique<SubprogramDeclNode>(name, build(blockNode));
}
unique_ptr<ASTNode> AST_Tree::buildFieldAccess(Node* cstNode) {
    string recordName = extractVarName(cstNode->children[0].get());
    string fieldName = extractVarName(cstNode->children[2].get());
    return make_unique<FieldAccessNode>(recordName, fieldName);
}

unique_ptr<ASTNode> AST_Tree::buildEmptyStatement(Node* cstNode) {
    return nullptr; 
}