#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ASTBuilder.h"
#include "SymbolTableBuilder.h"
#include <string>
#include <map>
#include <vector>
#include <stack>

// Value type for runtime values
struct Value {
    enum Type { INT, CHAR, STRING, BOOL, ARRAY } type;
    int intVal;
    char charVal;
    std::string strVal;
    bool boolVal;
    std::vector<Value> arrayVal;
    
    Value() : type(INT), intVal(0), charVal(0), boolVal(false) {}
    explicit Value(int v) : type(INT), intVal(v), charVal(0), boolVal(false) {}
    explicit Value(char v) : type(CHAR), intVal(0), charVal(v), boolVal(false) {}
    explicit Value(const std::string& v) : type(STRING), intVal(0), charVal(0), strVal(v), boolVal(false) {}
    explicit Value(bool v) : type(BOOL), intVal(0), charVal(0), boolVal(v) {}
};

class Interpreter {
public:
    Interpreter(ASTNode* ast, SymbolTable& symTable, const std::vector<ParameterList>& paramLists);
    void execute();
    
private:
    ASTNode* astRoot;
    SymbolTable& symbolTable;
    const std::vector<ParameterList>& parameterLists;
    std::map<std::string, Value> variables;  // variable name -> value
    std::map<std::string, ASTNode*> routines; // function/procedure name -> routine node
    bool returnFlag;
    Value returnValue;
    
    // Execution methods
    void executeProgram(ASTNode* node);
    void executeRoutine(ASTNode* node, const std::vector<Value>& args);
    void executeBlock(ASTNode* node);
    void executeStatement(ASTNode* node);
    void executeAssignment(ASTNode* node);
    void executeIf(ASTNode* node);
    void executeWhile(ASTNode* node);
    void executeFor(ASTNode* node);
    void executeReturn(ASTNode* node);
    void executeCall(ASTNode* node);
    void executePrintf(ASTNode* node);
    
    // Expression evaluation
    Value evaluateExpression(ASTNode* node);
    Value evaluatePostfix(ASTNode* node);
    
    // Helper methods
    void initializeRoutines(ASTNode* node);
    void declareVariables(ASTNode* node);
    std::string getVariableName(ASTNode* node);
    void setVariable(const std::string& name, const Value& val);
    Value getVariable(const std::string& name);
    void setArrayElement(const std::string& name, int index, const Value& val);
    Value getArrayElement(const std::string& name, int index);
    int toInt(const Value& v);
    char toChar(const Value& v);
    std::string toString(const Value& v);
    bool toBool(const Value& v);
};

#endif
