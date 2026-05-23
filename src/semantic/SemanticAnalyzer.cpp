#include "semantic/SemanticAnalyzer.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>

static const bool DEBUG_SEMANTIC = false;

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& table) 
    : symbolTable(table), currentLev(0), currentBlock(0) {}

void SemanticAnalyzer::reportError(const std::string& message) {
    errors.push_back("Semantic Error: " + message);
}

bool SemanticAnalyzer::hasError() const {
    return !errors.empty();
}

void SemanticAnalyzer::printErrors() const {
    for (const auto& err : errors) {
        std::cerr << err << std::endl;
    }
}

bool SemanticAnalyzer::isDeclared(const std::string& name) const {
    int idx = symbolTable.lookupIndex(name);
    if (idx > 0) {
        auto entry = symbolTable.getTabEntry(idx);
        return entry.lev == currentLev; 
    }
    return false;
}

const std::vector<std::string>& SemanticAnalyzer::getErrors() const {
    return errors;
}

std::string SemanticAnalyzer::extractRawValue(const std::string& formattedLexeme) const {
    size_t start = formattedLexeme.find('(');
    size_t end = formattedLexeme.rfind(')');
    if (start != std::string::npos && end != std::string::npos && start < end) {
        return formattedLexeme.substr(start + 1, end - start - 1);
    }
    return formattedLexeme;
}
DataType SemanticAnalyzer::resolveTypeFromNode(Node* typeNode) const {
    if (!typeNode) return TYPE_NONE;

    if (typeNode->type == ARRAY_TYPE) return TYPE_ARRAY;
    if (typeNode->type == RECORD_TYPE) return TYPE_RECORD;

    if (!typeNode->children.empty()) {
        if (typeNode->children[0]->type == ARRAY_TYPE) return TYPE_ARRAY;
        if (typeNode->children[0]->type == RECORD_TYPE) return TYPE_RECORD;
    }

    std::string raw = typeNode->lexeme;
    if (raw.empty() && !typeNode->children.empty()) {
        raw = typeNode->children[0]->lexeme;
    }

    std::string typeName = extractRawValue(raw);

    if (typeName.find("integer") != std::string::npos) return TYPE_INTEGER;
    if (typeName.find("real") != std::string::npos)    return TYPE_REAL;
    if (typeName.find("char") != std::string::npos)    return TYPE_CHAR;
    if (typeName.find("boolean") != std::string::npos) return TYPE_BOOLEAN;
    if (typeName.find("string") != std::string::npos)  return TYPE_STRING;
    if (typeName.find("array") != std::string::npos)   return TYPE_ARRAY;
    if (typeName.find("record") != std::string::npos)  return TYPE_RECORD;

    int index = symbolTable.lookupIndex(typeName);
    if (index > 0) {
        auto entry = symbolTable.getTabEntry(index);
        if (entry.obj == OBJ_TYPE) {
            return entry.type;
        }
    }

    return TYPE_NONE;
}

int SemanticAnalyzer::buildArrayEntry(Node* arrayTypeNode) {
    if (!arrayTypeNode || arrayTypeNode->type != ARRAY_TYPE) return 0;

    Node* boundsNode = arrayTypeNode->children[2].get(); 
    Node* elementTypeNode = arrayTypeNode->children[5].get();

    int lowBound = 0;
    int highBound = 0;
    DataType indexType = TYPE_INTEGER; 

    if (boundsNode->type == RANGE) {
        std::string lowStr = extractRawValue(boundsNode->children[0]->lexeme);
        std::string highStr = extractRawValue(boundsNode->children[3]->lexeme);
        
        try {
            lowBound = std::stoi(lowStr.empty() ? "0" : lowStr);
            highBound = std::stoi(highStr.empty() ? "0" : highStr);
        } catch (...) {
            lowBound = 0; highBound = 0;
        }
    }

    if (lowBound > highBound && boundsNode->type == RANGE) {
        reportError("Range array tidak valid karena batas bawah lebih besar dari batas atas.");
    }

    DataType elType = resolveTypeFromNode(elementTypeNode);
    int elementSize = 1; 
    int totalSize = (highBound - lowBound + 1) * elementSize;

    return symbolTable.buildAtabEntryDirect(indexType, elType, 0, lowBound, highBound, elementSize, (totalSize > 0 ? totalSize : 0)); 
}

void SemanticAnalyzer::analyze(Node* cstRoot) {
    currentLev = 0;
    currentBlock = 0;
    
    symbolTable.resetBlockZero(); 
    
    traverseNode(cstRoot, currentLev);
}

void SemanticAnalyzer::traverseNode(Node* node, int lev) {
    if (!node) return;

    int prevLev = currentLev;
    currentLev = lev;

    switch (node->type) {
        case PROGRAM: {
            std::string progName = "unknown";
            if (!node->children.empty() && node->children[0]->type == PROGRAM_HEADER) {
                if (node->children[0]->children.size() > 1) {
                    progName = extractRawValue(node->children[0]->children[1]->lexeme);
                }
            }
            
            int idx = symbolTable.addEntry(progName, OBJ_PROCEDURE, TYPE_NONE, 0, 1, lev, 0);
            currentBlock = symbolTable.pushNewBlock(idx); 

            for (auto& child : node->children) {
                traverseNode(child.get(), lev);
            }
            break;
        }
        case CONST_DECLARATION: {
            size_t i = 1;
            while (i + 2 < node->children.size()) {
                Node* idNode = node->children[i].get();
                Node* constNode = node->children[i+2].get();
                
                if (idNode->type == TOKEN_NODE) {
                    std::string constName = extractRawValue(idNode->lexeme);
                   if (isDeclared(constName)) {
                        reportError("Konstanta '" + constName + "' sudah dideklarasikan pada scope ini.");
                        i += 4;
                        continue;
                    }
                    DataType constType = TYPE_NONE;
                    
                    if (!constNode->children.empty()) {
                        std::string valLexeme = constNode->children.back()->lexeme;
                        if (valLexeme.find("intcon") != std::string::npos) constType = TYPE_INTEGER;
                        else if (valLexeme.find("realcon") != std::string::npos) constType = TYPE_REAL;
                        else if (valLexeme.find("charcon") != std::string::npos) constType = TYPE_CHAR;
                        else if (valLexeme.find("string_tok") != std::string::npos) constType = TYPE_STRING;
                    }
                    
                    int idx = symbolTable.addEntry(constName, OBJ_CONSTANT, constType, 0, 1, lev, 0);
                    symbolTable.updateCurrentBlockLast(idx);
                }
                i += 4;
            }
            break;
        }
        case TYPE_DECLARATION: {
            size_t i = 1;
            while (i + 2 < node->children.size()) {
                Node* idNode = node->children[i].get();
                Node* typeNode = node->children[i+2].get();
                
                if (idNode->type == TOKEN_NODE) {
                    std::string typeName = extractRawValue(idNode->lexeme);
                    if (isDeclared(typeName)) {
                        reportError("Tipe data '" + typeName + "' sudah dideklarasikan pada scope ini.");
                        i += 4;
                        continue;
                    }

                    DataType resolvedType = resolveTypeFromNode(typeNode);

                    if (resolvedType == TYPE_NONE) {
                        if (typeNode->type == RANGE || (!typeNode->children.empty() && typeNode->children[0]->type == RANGE)) {
                            resolvedType = TYPE_INTEGER; 
                        } else if (typeNode->type == ENUMERATED || (!typeNode->children.empty() && typeNode->children[0]->type == ENUMERATED)) {
                            resolvedType = TYPE_INTEGER; 
                        }
                    }

                    if (resolvedType == TYPE_NONE) {
                        reportError("Tipe data '" + typeName + "' tidak dikenali.");
                    }
                    int typeRef = 0;

                    Node* actualType = typeNode;
                    if (!typeNode->children.empty()) {
                        actualType = typeNode->children[0].get();
                    }

                    if (resolvedType == TYPE_ARRAY && actualType->type == ARRAY_TYPE) {
                        typeRef = buildArrayEntry(actualType);
                    }

                    int idx = symbolTable.addEntry(typeName, OBJ_TYPE, resolvedType, typeRef, 1, lev, 0);
                    symbolTable.updateCurrentBlockLast(idx);
                }
                i += 4;
            }
            break;
        }
        case VAR_DECLARATION: {
            size_t i = 1;
            while (i + 2 < node->children.size()) {
                Node* idListNode = node->children[i].get();
                Node* typeNode = node->children[i + 2].get();
                DataType resolvedType = resolveTypeFromNode(typeNode);
                int typeRef = 0;
                if (resolvedType == TYPE_ARRAY) {
                    Node* actualType = typeNode;
                    if (actualType && !actualType->children.empty()) actualType = actualType->children[0].get();
                    if (actualType && actualType->type == ARRAY_TYPE) {
                        typeRef = buildArrayEntry(actualType);
                    }
                }
                if ((resolvedType == TYPE_ARRAY || resolvedType == TYPE_RECORD) && typeRef == 0) {
                    std::string raw = typeNode->lexeme;
                    if (raw.empty() && !typeNode->children.empty()) raw = typeNode->children[0]->lexeme;
                    std::string tName = extractRawValue(raw);
                    
                    int tIdx = symbolTable.lookupIndex(tName);
                    if (tIdx > 0) {
                        auto entry = symbolTable.getTabEntry(tIdx);
                        if (entry.obj == OBJ_TYPE) typeRef = entry.ref;
                    }
                }
                for (auto& identNode : idListNode->children) {
                    if (identNode->type == TOKEN_NODE && identNode->lexeme.find("ident") != std::string::npos) {
                        std::string varName = extractRawValue(identNode->lexeme);

                        if (isDeclared(varName)) {
                            reportError("Variabel '" + varName + "' sudah dideklarasikan pada scope ini.");
                            continue;
                        }
                        int idx = symbolTable.addEntry(varName, OBJ_VARIABLE, resolvedType, typeRef, 1, lev, 0);
                        symbolTable.incrementCurrentBlockVsze(idx);
                    }
                }
                i += 4;
            }
            break;
        }


        case FUNCTION_DECLARATION:
        case PROCEDURE_DECLARATION: {
            std::string subName = extractRawValue(node->children[1]->lexeme);
            ObjectClass objClass = (node->type == FUNCTION_DECLARATION) ? OBJ_FUNCTION : OBJ_PROCEDURE;
            DataType retType = TYPE_NONE;

            if (objClass == OBJ_FUNCTION) {
                for (size_t i = 0; i < node->children.size(); ++i) {
                    if (node->children[i]->lexeme == "colon") {
                        retType = resolveTypeFromNode(node->children[i+1].get());
                        break;
                    }
                }
            }

            int procIdx = symbolTable.addEntry(subName, objClass, retType, 0, 1, lev, 0);
            symbolTable.updateCurrentBlockLast(procIdx);

            int savedBlock = currentBlock;
            currentBlock = symbolTable.pushNewBlock(procIdx);

            int paramCount = 0;
            for (auto& child : node->children) {
                if (child->type == FORMAL_PARAMETER_LIST) {
                    for (auto& pGroup : child->children) {
                        if (pGroup->type == PARAMETER_GROUP) {
                            Node* identList = pGroup->children[0].get();
                            Node* typeNode = pGroup->children[2].get();
                            DataType paramType = resolveTypeFromNode(typeNode);
                            
                            int pRef = 0;
                            Node* pActualType = typeNode;
                            if (!typeNode->children.empty()) pActualType = typeNode->children[0].get();
                            if (paramType == TYPE_ARRAY && pActualType->type == ARRAY_TYPE) {
                                pRef = buildArrayEntry(pActualType);
                            } else if (paramType == TYPE_ARRAY || paramType == TYPE_RECORD) {
                                std::string pTName = extractRawValue(typeNode->lexeme);
                                if (pTName.empty() && !typeNode->children.empty()) pTName = extractRawValue(typeNode->children[0]->lexeme);
                                int pTIdx = symbolTable.lookupIndex(pTName);
                                if (pTIdx > 0) {
                                    auto pEntry = symbolTable.getTabEntry(pTIdx);
                                    if (pEntry.obj == OBJ_TYPE) pRef = pEntry.ref;
                                }
                            }

                            for (auto& leaf : identList->children) {
                                if (leaf->lexeme.find("ident(") == 0) {
                                    std::string paramName = extractRawValue(leaf->lexeme);
                                    int idx = symbolTable.addEntry(paramName, OBJ_VARIABLE, paramType, pRef, 1, lev + 1, 0);
                                    symbolTable.incrementCurrentBlockVsze(idx);
                                    paramCount++;
                                }
                            }
                        }
                    }
                }
            }
            symbolTable.updateCurrentBlockLpar(paramCount);

            for (auto& child : node->children) {
                if (child->type == BLOCK) traverseNode(child.get(), lev + 1);
            }

            currentBlock = savedBlock; 
            break;
        }
        default: {
            for (auto& child : node->children) {
                traverseNode(child.get(), lev);
            }
            break;
        }
    }

    currentLev = prevLev; 
}

void SemanticAnalyzer::analyzeAST(ASTNode* astRoot) {
    if (!astRoot) return;
    checkTypeCompatibility(astRoot);
}

DataType SemanticAnalyzer::getExprType(ASTNode* exprNode) {
    if (!exprNode) return TYPE_NONE;

    if (auto numNode = dynamic_cast<NumberNode*>(exprNode)) {
        if (numNode->value.find('.') != std::string::npos) return TYPE_REAL;
        return TYPE_INTEGER;
    }
    
    if (auto strNode = dynamic_cast<StringNode*>(exprNode)) {
        return TYPE_STRING;
    }

    if (auto varNode = dynamic_cast<VariableNode*>(exprNode)) {
        int idx = symbolTable.lookupIndex(varNode->name);
        if (idx > 0) {
            auto entry = symbolTable.getTabEntry(idx);
            if (entry.obj == OBJ_VARIABLE || entry.obj == OBJ_FUNCTION || entry.obj == OBJ_CONSTANT) {
                return entry.type;
            }
        }
        reportError("Identifier '" + varNode->name + "' belum dideklarasikan atau tidak valid di konteks ini.");
        return TYPE_NONE;
    }

    if (auto arrNode = dynamic_cast<ArrayAccessNode*>(exprNode)) {
        int idx = symbolTable.lookupIndex(arrNode->arrayName);
        if (idx > 0) {
            auto entry = symbolTable.getTabEntry(idx);
            if (entry.type == TYPE_ARRAY && entry.ref > 0) {
                return symbolTable.getAtabEntry(entry.ref).etyp;
            }
        }
        reportError("Akses array ilegal pada '" + arrNode->arrayName + "'.");
        return TYPE_NONE;
    }

    if (auto fieldNode = dynamic_cast<FieldAccessNode*>(exprNode)) {
        // Implementasi sederhana: Asumsikan lookup nama record bisa memisahkan dot notation.
        // Untuk arsitektur ini, kita coba temukan parent recordnya
        size_t dotPos = fieldNode->recordName.find('.');
        std::string baseRec = (dotPos != std::string::npos) ? fieldNode->recordName.substr(0, dotPos) : fieldNode->recordName;
        int idx = symbolTable.lookupIndex(baseRec);
        if (idx <= 0) reportError("Record '" + baseRec + "' belum dideklarasikan.");
        return TYPE_NONE;
    }

    if (auto binOp = dynamic_cast<BinOpNode*>(exprNode)) {
        DataType leftType = getExprType(binOp->left.get());
        DataType rightType = getExprType(binOp->right.get());
        if (binOp->op == "==" || binOp->op == "<>" || binOp->op == "<" || 
            binOp->op == "<=" || binOp->op == ">" || binOp->op == ">=") {
            return TYPE_BOOLEAN;
        }
        if (binOp->op == "AND" || binOp->op == "OR") {
            if (leftType != TYPE_BOOLEAN || rightType != TYPE_BOOLEAN) {
                reportError("Operator '" + binOp->op + "' membutuhkan operand Boolean.");
            }
            return TYPE_BOOLEAN;
        }
        if (binOp->op == "MOD" || binOp->op == "div") {
            if (leftType != TYPE_INTEGER || rightType != TYPE_INTEGER) {
                reportError("Operator '" + binOp->op + "' membutuhkan operand Integer.");
            }
            return TYPE_INTEGER;
        }
        
        if (binOp->op == "/") {
            return TYPE_REAL;
        }

        if (binOp->op == "+" || binOp->op == "-" || binOp->op == "*") {
            if (leftType == TYPE_REAL || rightType == TYPE_REAL) return TYPE_REAL;
            return TYPE_INTEGER;
        }
    }
    
    if (auto callNode = dynamic_cast<CallNode*>(exprNode)) {
        int idx = symbolTable.lookupIndex(callNode->functionName); // Diperbaiki
        if (idx > 0) return symbolTable.getTabEntry(idx).type;
    }

    return TYPE_NONE;
}

void SemanticAnalyzer::checkTypeCompatibility(ASTNode* astNode) {
    if (!astNode) return;

    if (auto blockNode = dynamic_cast<BlockNode*>(astNode)) {
        for (auto& stmt : blockNode->statements) {
            checkTypeCompatibility(stmt.get());
        }
    }
    else if (auto assignNode = dynamic_cast<AssignNode*>(astNode)) {
        DataType targetType = getExprType(assignNode->targetVariable.get()); 
        DataType valType = getExprType(assignNode->value.get());
        
        if (targetType != TYPE_NONE && valType != TYPE_NONE) {
            if (targetType == TYPE_REAL && valType == TYPE_INTEGER) {
            } 
            else if (targetType == TYPE_RECORD || valType == TYPE_RECORD || 
                     targetType == TYPE_ARRAY || valType == TYPE_ARRAY) {
            } 
            else if (targetType != valType) {
                reportError("Type mismatch dalam assignment. Tidak dapat menetapkan nilai ke target.");
            }
        }
    }
    else if (auto ifNode = dynamic_cast<IfNode*>(astNode)) {
        DataType condType = getExprType(ifNode->condition.get());
        if (condType != TYPE_BOOLEAN && condType != TYPE_NONE) {
            reportError("Kondisi pada If-Statement harus berupa tipe Boolean.");
        }
        checkTypeCompatibility(ifNode->thenBranch.get());
        if (ifNode->elseBranch) checkTypeCompatibility(ifNode->elseBranch.get());
    }
    else if (auto whileNode = dynamic_cast<WhileNode*>(astNode)) {
        DataType condType = getExprType(whileNode->condition.get());
        if (condType != TYPE_BOOLEAN && condType != TYPE_NONE) {
            reportError("Kondisi pada While-Statement harus berupa tipe Boolean.");
        }
        checkTypeCompatibility(whileNode->body.get());
    }
    else if (auto repeatNode = dynamic_cast<RepeatNode*>(astNode)) {
        checkTypeCompatibility(repeatNode->body.get());
        DataType condType = getExprType(repeatNode->condition.get());
        if (condType != TYPE_BOOLEAN && condType != TYPE_NONE) {
            reportError("Kondisi pada Repeat-Statement harus berupa tipe Boolean.");
        }
    }
    else if (auto forNode = dynamic_cast<ForNode*>(astNode)) {
        DataType startType = getExprType(forNode->start.get());
        DataType endType = getExprType(forNode->end.get());
        if (startType != TYPE_INTEGER || endType != TYPE_INTEGER) {
            reportError("Batas perulangan For-Statement harus berupa tipe Integer.");
        }
        checkTypeCompatibility(forNode->body.get());
    }
    else if (auto subprogNode = dynamic_cast<SubprogramDeclNode*>(astNode)) {
        checkTypeCompatibility(subprogNode->block.get());
    }
}