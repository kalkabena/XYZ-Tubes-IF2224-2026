#include "syntax/AST_Tree.hpp"
#include <stdexcept>
#include <cctype>

unique_ptr<ASTNode> AST_Tree::build(Node* cstRoot) {
    if (!cstRoot) return nullptr;

    if (cstRoot->type == INDEX_LIST) {
        if (!cstRoot->children.empty()) return buildExpression(cstRoot->children[0].get());
    }

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

    // Safety Net
    for (auto& child : cstRoot->children) {
        auto res = build(child.get());
        if (res) return res; 
    }

    return nullptr;
}

unique_ptr<ASTNode> AST_Tree::buildComponentVariable(Node* cstNode) {
    if (cstNode->children.size() >= 3 && cstNode->children[1]->lexeme.find("period") != string::npos) {
        string recordName = extractVarName(cstNode->children[0].get());
        string fieldName = extractVarName(cstNode->children[2].get());
        return make_unique<FieldAccessNode>(recordName, fieldName);
    }
    string varName = extractVarName(cstNode);
    return make_unique<VariableNode>(varName);
}

unique_ptr<ASTNode> AST_Tree::buildAssignment(Node* cstNode) {
    Node* varNode = cstNode->children[0].get(); 
    Node* exprNode = cstNode->children[2].get(); 

    string baseName = extractVarName(varNode->children[0].get());
    unique_ptr<ASTNode> targetAST = make_unique<VariableNode>(baseName);

    for (size_t i = 1; i < varNode->children.size(); ++i) {
        Node* compNode = varNode->children[i].get();
        if (compNode->type == COMPONENT_VARIABLE && !compNode->children.empty()) {
            Node* firstTok = compNode->children[0].get();
            
            if (firstTok->lexeme.find("lbrack") != string::npos && compNode->children.size() > 1) {
                Node* idxList = compNode->children[1].get();
                if (!idxList->children.empty()) {
                    auto idxExpr = buildExpression(idxList->children[0].get());
                    if (!idxExpr) idxExpr = make_unique<VariableNode>(extractVarName(idxList->children[0].get()));
                    targetAST = make_unique<ArrayAccessNode>(baseName, move(idxExpr));
                }
            } else if (firstTok->lexeme.find("period") != string::npos && compNode->children.size() > 1) {
                string field = extractVarName(compNode->children[1].get());
                targetAST = make_unique<FieldAccessNode>(baseName, field);
                baseName = baseName + "." + field; 
            }
        }
    }

    return make_unique<AssignNode>(move(targetAST), buildExpression(exprNode));
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
        
        // Ekstraksi operator secara akurat menembus daun node
        Node* opNode = cstNode->children[1].get();
        string opStr = extractVarName(opNode); 
        if (!opNode->children.empty()) {
            string rawOp = opNode->children[0]->lexeme;
            if (rawOp.find("plus") != string::npos) opStr = "+";
            else if (rawOp.find("minus") != string::npos) opStr = "-";
            else if (rawOp.find("times") != string::npos) opStr = "*";
            else if (rawOp.find("div") != string::npos) opStr = "/";
            else if (rawOp.find("mod") != string::npos) opStr = "MOD";
            else if (rawOp.find("eql") != string::npos) opStr = "==";
            else if (rawOp.find("neq") != string::npos) opStr = "<>";
            else if (rawOp.find("gtr") != string::npos) opStr = ">";
            else if (rawOp.find("geq") != string::npos) opStr = ">=";
            else if (rawOp.find("lss") != string::npos) opStr = "<";
            else if (rawOp.find("leq") != string::npos) opStr = "<=";
            else if (rawOp.find("and") != string::npos) opStr = "AND";
            else if (rawOp.find("or") != string::npos) opStr = "OR";
        }
        return make_unique<BinOpNode>(opStr, buildExpression(cstNode->children[0].get()), buildExpression(cstNode->children[2].get()));
    }

    if (cstNode->type == FACTOR) {
        Node* primary = cstNode->children[0].get();
        
        if (primary->lexeme.find("lparent") != string::npos) {
            if (cstNode->children.size() > 1) return buildExpression(cstNode->children[1].get());
        }
    
        if (primary->type == PROCEDURE_FUNCTION_CALL) {
            return buildFunctionCall(primary);
        }
        if (primary->type == VARIABLE) {
            string baseName = extractVarName(primary->children[0].get());
            unique_ptr<ASTNode> currentVar = make_unique<VariableNode>(baseName);
            
            for (size_t i = 1; i < primary->children.size(); ++i) {
                Node* comp = primary->children[i].get();
                if (comp->type == COMPONENT_VARIABLE && !comp->children.empty()) {
                    Node* firstTok = comp->children[0].get();
                    if (firstTok->lexeme.find("lbrack") != string::npos && comp->children.size() > 1) {
                        Node* idxList = comp->children[1].get();
                        if (!idxList->children.empty()) {
                            auto idxExpr = buildExpression(idxList->children[0].get());
                            if (!idxExpr) idxExpr = make_unique<VariableNode>(extractVarName(idxList->children[0].get()));
                            currentVar = make_unique<ArrayAccessNode>(baseName, move(idxExpr));
                        }
                    } else if (firstTok->lexeme.find("period") != string::npos && comp->children.size() > 1) {
                        string field = extractVarName(comp->children[1].get());
                        currentVar = make_unique<FieldAccessNode>(baseName, field);
                        baseName = baseName + "." + field; 
                    }
                }
            }
            return currentVar;
        }

        if (primary->type == TOKEN_NODE) {
            string rawVal = extractVarName(primary);
            if (primary->lexeme.find("string_tok") != string::npos || (!rawVal.empty() && rawVal[0] == '\'')) {
                return make_unique<StringNode>(rawVal);
            }
            if (primary->lexeme.find("intcon") != string::npos || primary->lexeme.find("realcon") != string::npos || (!rawVal.empty() && isdigit(rawVal[0]))) {
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