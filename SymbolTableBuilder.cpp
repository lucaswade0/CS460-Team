#include "SymbolTableBuilder.h"
#include <iostream>

SymbolTableEntry::SymbolTableEntry(std::string name, std::string idType, std::string dtype, bool array, int arrSize, int sc, int ln)
    : identifierName(name), identifierType(idType), dataType(dtype),
      isArray(array), arraySize(arrSize), scope(sc), line(ln), next(nullptr) {}

SymbolTable::SymbolTable() : head(nullptr), tail(nullptr) {}

SymbolTableEntry* SymbolTable::findInScope(std::string name, int scope) {
    SymbolTableEntry* curr = head;
    while (curr) {
        if (curr->identifierName == name && curr->scope == scope) {
            return curr;
        }
        curr = curr->next;
    }
    return nullptr;
}

void SymbolTable::insert(std::string name, std::string idType, std::string dtype, bool isArray, int arrSize, int scope, int line) {
    SymbolTableEntry* entry = new SymbolTableEntry(name, idType, dtype, isArray, arrSize, scope, line);

    if (!head) {
        head = tail = entry;
    } else {
        tail->next = entry;
        tail = entry;
    }
}

void SymbolTable::print() {
    SymbolTableEntry* curr = head;
    while (curr) {
        if (curr->identifierType != "parameter") {
            std::cout << "      IDENTIFIER_NAME: " << curr->identifierName << std::endl;
            std::cout << "      IDENTIFIER_TYPE: " << curr->identifierType << std::endl;
            std::cout << "             DATATYPE: " << curr->dataType << std::endl;
            std::cout << "    DATATYPE_IS_ARRAY: ";
            if (curr->isArray) {
                std::cout << "yes";
            } else {
                std::cout << "no";
            }
            std::cout << std::endl;
            std::cout << "  DATATYPE_ARRAY_SIZE: " << curr->arraySize << std::endl;
            std::cout << "                SCOPE: " << curr->scope << std::endl;
            std::cout << std::endl;
        }
        curr = curr->next;
    }
}

void SymbolTableBuilder::buildSymbolTable(TreeNode* node, SymbolTable& table, int& currentScope,
                                          std::vector<ParameterList>& parameterLists) {
    if (!node) return;

    if (node->value == "function" && node->leftChild && node->leftChild->value == "function") {
        TreeNode* kw = node->leftChild;
        TreeNode* typeNode = kw->rightSibling;
        TreeNode* nameNode;
        if (typeNode) {
            nameNode = typeNode->rightSibling;
        } else {
            nameNode = nullptr;
        }
        if (!kw || !typeNode || !nameNode) {
            return;
        }

        std::string funcName = nameNode->value;
        std::string funcType = typeNode->value;

        currentScope++;
        int funcScope = currentScope;

        table.insert(funcName, "function", funcType, false, 0, funcScope, nameNode->line);

        TreeNode* walker = nameNode->rightSibling;
        while (walker && walker->value != "Parameters") {
            walker = walker->rightSibling;
        }

        ParameterList paramList;
        paramList.functionName = funcName;
        if (walker) {
            TreeNode* param = walker->leftChild;
            while (param) {
                if (param->value == "Parameter") {
                    std::string type = param->leftChild->value;
                    TreeNode* paramNameNode = param->leftChild->rightSibling;
                    std::string paramName = paramNameNode->value;
                    int paramLine = paramNameNode->line;
                    bool isArray = false;
                    int arraySize = 0;

                    TreeNode* bracketNode = param->leftChild->rightSibling->rightSibling;
                    if (bracketNode && bracketNode->value == "[") {
                        isArray = true;
                        TreeNode* sizeNode = bracketNode->rightSibling;
                        if (sizeNode && sizeNode->value != "]") {
                            arraySize = std::stoi(sizeNode->value);
                        }
                    }

                    paramList.params.push_back({paramName, type, funcScope, isArray, arraySize});
                    table.insert(paramName, "parameter", type, isArray, arraySize, funcScope, paramLine);
                }
                param = param->rightSibling;
            }
        }
        parameterLists.push_back(paramList);

        TreeNode* block = walker;
        while (block && block->value != "Block") {
            block = block->rightSibling;
        }
        if (block) {
            buildSymbolTable(block, table, funcScope, parameterLists);
        }

        if (node->rightSibling) {
            buildSymbolTable(node->rightSibling, table, currentScope, parameterLists);
        }
        return;
    }

    if (node->value == "procedure" && node->leftChild && node->leftChild->value == "procedure") {
        TreeNode* kw = node->leftChild;
        TreeNode* nameNode = kw->rightSibling;
        if (!nameNode) {
            return;
        }
        std::string procName = nameNode->value;

        currentScope++;
        int procScope = currentScope;

        table.insert(procName, "procedure", "NOT APPLICABLE", false, 0, procScope, nameNode->line);

        TreeNode* walker = nameNode->rightSibling;
        while (walker && walker->value != "Parameters") {
            walker = walker->rightSibling;
        }

        ParameterList paramList;
        paramList.functionName = procName;
        if (walker) {
            TreeNode* param = walker->leftChild;
            while (param) {
                if (param->value == "Parameter") {
                    std::string type = param->leftChild->value;
                    TreeNode* paramNameNode = param->leftChild->rightSibling;
                    std::string paramName = paramNameNode->value;
                    int paramLine = paramNameNode->line;
                    bool isArray = false;
                    int arraySize = 0;

                    TreeNode* bracketNode = param->leftChild->rightSibling->rightSibling;
                    if (bracketNode && bracketNode->value == "[") {
                        isArray = true;
                        TreeNode* sizeNode = bracketNode->rightSibling;
                        if (sizeNode && sizeNode->value != "]") {
                            arraySize = std::stoi(sizeNode->value);
                        }
                    }

                    paramList.params.push_back({paramName, type, procScope, isArray, arraySize});
                    table.insert(paramName, "parameter", type, isArray, arraySize, procScope, paramLine);
                }
                param = param->rightSibling;
            }
        }

        if (!paramList.params.empty()) {
            parameterLists.push_back(paramList);
        }

        TreeNode* block = walker;
        while (block && block->value != "Block") {
            block = block->rightSibling;
        }
        if (block) {
            buildSymbolTable(block, table, procScope, parameterLists);
        }

        if (node->rightSibling) {
            buildSymbolTable(node->rightSibling, table, currentScope, parameterLists);
        }
        return;
    }

    if (node->value == "Declaration" || node->value == "GlobalDecl") {
        std::string type = node->leftChild->value;
        int typeLine = node->leftChild->line;
        TreeNode* var = node->leftChild->rightSibling;
        while (var) {
            if (var->value == "VarDecl") {
                std::string name = var->leftChild->value;
                int varLine = var->leftChild->line;

                int lineToReport;
                if (varLine > 0) {
                    lineToReport = varLine;
                } else {
                    lineToReport = typeLine;
                }

                bool isArray = var->leftChild->rightSibling && var->leftChild->rightSibling->value == "[";
                int size;
                if (isArray) {
                    size = std::stoi(var->leftChild->rightSibling->rightSibling->value);
                } else {
                    size = 0;
                }

                int scopeToUse;
                if (node->value == "GlobalDecl") {
                    scopeToUse = 0;
                } else {
                    scopeToUse = currentScope;
                }

                SymbolTableEntry* existingVar = table.findInScope(name, scopeToUse);
                if (existingVar && (existingVar->identifierType == "datatype" || existingVar->identifierType == "parameter")) {
                    if (scopeToUse == 0) {
                        std::cerr << "Error on line " << lineToReport << ": variable \"" << name
                             << "\" is already defined globally" << std::endl;
                    } else {
                        std::cerr << "Error on line " << lineToReport << ": variable \"" << name
                             << "\" is already defined locally" << std::endl;
                    }
                    exit(1);
                }

                if (scopeToUse > 0) {
                    SymbolTableEntry* globalVar = table.findInScope(name, 0);
                    if (globalVar && (globalVar->identifierType == "datatype" || globalVar->identifierType == "parameter")) {
                        std::cerr << "Error on line " << lineToReport << ": variable \"" << name
                             << "\" is already defined globally" << std::endl;
                        exit(1);
                    }
                }

                table.insert(name, "datatype", type, isArray, size, scopeToUse, lineToReport);
            }
            var = var->rightSibling;
        }
    }

    if (node->leftChild)
        buildSymbolTable(node->leftChild, table, currentScope, parameterLists);

    if (node->rightSibling)
        buildSymbolTable(node->rightSibling, table, currentScope, parameterLists);
}

void SymbolTableBuilder::printParameterLists(const std::vector<ParameterList>& parameterLists) {
    for (const auto& paramList : parameterLists) {
        std::cout << std::endl;
        std::cout << "   PARAMETER LIST FOR: " << paramList.functionName << std::endl;
        for (const auto& param : paramList.params) {
            std::cout << "      IDENTIFIER_NAME: " << std::get<0>(param) << std::endl;
            std::cout << "             DATATYPE: " << std::get<1>(param) << std::endl;
            std::cout << "    DATATYPE_IS_ARRAY: ";
            if (std::get<3>(param)) {
                std::cout << "yes";
            } else {
                std::cout << "no";
            }
            std::cout << std::endl;
            std::cout << "  DATATYPE_ARRAY_SIZE: " << std::get<4>(param) << std::endl;
            std::cout << "                SCOPE: " << std::get<2>(param) << std::endl;
            std::cout << std::endl;
        }
    }
}
