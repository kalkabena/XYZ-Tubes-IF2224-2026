#include "SemanticAnalyzer.hpp"
#include <stdexcept>

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& table) 
    : symbolTable(table), currentLev(0), currentBlock(0) {}

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

    if (!typeNode->children.empty()) {
        if (typeNode->children[0]->type == ARRAY_TYPE) return TYPE_ARRAY;
        if (typeNode->children[0]->type == RECORD_TYPE) return TYPE_RECORD;
    }

    std::string typeName = extractRawValue(typeNode->lexeme);

    if (typeName == "integer") return TYPE_INTEGER;
    if (typeName == "real")    return TYPE_REAL;
    if (typeName == "char")    return TYPE_CHAR;
    if (typeName == "boolean") return TYPE_BOOLEAN;
    if (typeName == "string")  return TYPE_STRING;

    if (typeName == "array")  return TYPE_ARRAY;
    if (typeName == "record") return TYPE_RECORD;

    // Panggil lookupIndex via referensi symbolTable
    int index = symbolTable.lookupIndex(typeName);
    if (index > 0) {
        // Asumsi lu sudah menyediakan fungsi publik getTabEntry(index) di SymbolTable
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

    switch (node->type) {
        case PROGRAM: {
            std::string progName = "unknown";
            if (!node->children.empty() && node->children[0]->type == PROGRAM_HEADER) {
                if (node->children[0]->children.size() > 1) {
                    progName = extractRawValue(node->children[0]->children[1]->lexeme);
                }
            }
            
            int idx = symbolTable.addEntry(progName, OBJ_PROCEDURE, TYPE_NONE, 0, 1, lev, 0);
            currentBlock = symbolTable.pushNewBlock(idx); // Kelola via SymbolTable

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
                    DataType resolvedType = resolveTypeFromNode(typeNode);
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
                Node* typeNode = node->children[i+2].get(); 
                
                DataType resolvedType = resolveTypeFromNode(typeNode);
                int typeRef = 0;
                
                Node* actualType = typeNode;
                if (!typeNode->children.empty()) {
                    actualType = typeNode->children[0].get();
                }

                if (resolvedType == TYPE_ARRAY && actualType->type == ARRAY_TYPE) {
                    typeRef = buildArrayEntry(actualType);
                } else if (resolvedType == TYPE_ARRAY || resolvedType == TYPE_RECORD) {
                    std::string tName = extractRawValue(typeNode->lexeme);
                    if (tName.empty() && !typeNode->children.empty()) {
                        tName = extractRawValue(typeNode->children[0]->lexeme);
                    }
                    int tIdx = symbolTable.lookupIndex(tName);
                    if (tIdx > 0) {
                        auto entry = symbolTable.getTabEntry(tIdx);
                        if (entry.obj == OBJ_TYPE) typeRef = entry.ref;
                    }
                }

                for (auto& identNode : idListNode->children) {
                    if (identNode->type == TOKEN_NODE && identNode->lexeme.find("ident") != std::string::npos) {
                        std::string varName = extractRawValue(identNode->lexeme);
                        
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

            currentBlock = savedBlock; // Amankan kembali konteks block parent
            break;
        }
        default: {
            for (auto& child : node->children) {
                traverseNode(child.get(), lev);
            }
            break;
        }
    }
}