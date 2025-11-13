#ifndef SYMBOLTABLEBUILDER_H
#define SYMBOLTABLEBUILDER_H

#include "CSTParser.h"
#include <string>
#include <vector>
#include <tuple>

struct SymbolTableEntry {
    std::string identifierName;
    std::string identifierType;
    std::string dataType;
    bool isArray;
    int arraySize;
    int scope;
    int line;
    SymbolTableEntry* next;

    SymbolTableEntry(std::string name, std::string idType, std::string dtype, bool array, int arrSize, int sc, int ln);
};

class SymbolTable {
public:
    SymbolTableEntry* head;
    SymbolTableEntry* tail;

    SymbolTable();
    SymbolTableEntry* findInScope(std::string name, int scope);
    void insert(std::string name, std::string idType, std::string dtype, bool isArray, int arrSize, int scope, int line);
    void print();
};

struct ParameterList {
    std::string functionName;
    std::vector<std::tuple<std::string, std::string, int, bool, int>> params;
};

class SymbolTableBuilder {
public:
    static void buildSymbolTable(TreeNode* node, SymbolTable& table, int& currentScope,
                                  std::vector<ParameterList>& parameterLists);
    static void printParameterLists(const std::vector<ParameterList>& parameterLists);
};

#endif
