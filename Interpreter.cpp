#include "Interpreter.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cctype>

using namespace std;

Interpreter::Interpreter(ASTNode* ast, SymbolTable& symTable, const vector<ParameterList>& paramLists)
    : astRoot(ast), symbolTable(symTable), parameterLists(paramLists), returnFlag(false) {
}

void Interpreter::execute() {
    if (!astRoot || astRoot->kind != "Program") {
        cerr << "Error: Invalid AST root" << endl;
        return;
    }
    executeProgram(astRoot);
}

void Interpreter::executeProgram(ASTNode* node) {
    // First pass: collect all routines
    initializeRoutines(node);
    
    // Debug: print found routines
    // for (auto& r : routines) {
    //     cerr << "Found routine: " << r.first << endl;
    // }
    
    // Find and execute main
    if (routines.find("main") == routines.end()) {
        cerr << "Error: main procedure not found" << endl;
        return;
    }
    
    vector<Value> emptyArgs;
    executeRoutine(routines["main"], emptyArgs);
}

void Interpreter::initializeRoutines(ASTNode* node) {
    // Get routine names from symbol table
    SymbolTableEntry* entry = symbolTable.head;
    vector<string> routineNames;
    while (entry) {
        if (entry->identifierType == "function" || entry->identifierType == "procedure") {
            routineNames.push_back(entry->identifierName);
        }
        entry = entry->next;
    }
    
    // Match routines with AST nodes in order
    ASTNode* child = node->leftChild;
    size_t routineIndex = 0;
    while (child && routineIndex < routineNames.size()) {
        if (child->kind == "Routine") {
            routines[routineNames[routineIndex]] = child;
            routineIndex++;
        }
        child = child->rightSibling;
    }
}

void Interpreter::executeRoutine(ASTNode* node, const vector<Value>& args) {
    if (!node || node->kind != "Routine") return;
    
    // Save current variable state (for nested calls)
    map<string, Value> savedVars = variables;
    
    // Get routine name from routines map (reverse lookup)
    string routineName;
    for (auto& r : routines) {
        if (r.second == node) {
            routineName = r.first;
            break;
        }
    }
    
    // Get parameter names from parameter lists
    vector<string> paramNames;
    for (const ParameterList& pl : parameterLists) {
        if (pl.functionName == routineName) {
            for (const auto& param : pl.params) {
                paramNames.push_back(get<0>(param));  // parameter name is first element
            }
            break;
        }
    }
    
    // Assign arguments to parameters
    // cerr << "DEBUG: Setting " << paramNames.size() << " parameters for " << routineName << endl;
    for (size_t i = 0; i < args.size() && i < paramNames.size(); i++) {
        setVariable(paramNames[i], args[i]);
        // cerr << "DEBUG: Set param " << paramNames[i] << " = " << toInt(args[i]) << " (char: '" << toChar(args[i]) << "')" << endl;
    }
    
    // Find and execute the block
    ASTNode* child = node->leftChild;
    while (child) {
        if (child->kind == "Block") {
            executeBlock(child);
            break;
        }
        child = child->rightSibling;
    }
    
    // Restore variables (except for main)
    if (routineName != "main") {
        variables = savedVars;
    }
}

void Interpreter::executeBlock(ASTNode* node) {
    if (!node || node->kind != "Block") return;
    
    ASTNode* child = node->leftChild;
    while (child && !returnFlag) {
        if (child->kind == "Decl") {
            declareVariables(child);
        } else {
            executeStatement(child);
        }
        child = child->rightSibling;
    }
}

void Interpreter::declareVariables(ASTNode* node) {
    if (!node || node->kind != "Decl") return;
    
    // Process all Var children
    ASTNode* varNode = node->leftChild;
    while (varNode) {
        if (varNode->kind == "Var") {
            string varName = varNode->text;
            
            // Check if it's an array by looking at symbol table
            SymbolTableEntry* entry = symbolTable.head;
            while (entry) {
                if (entry->identifierName == varName) {
                    if (entry->isArray) {
                        // Initialize array
                        Value arrVal;
                        arrVal.type = Value::ARRAY;
                        arrVal.arrayVal.resize(entry->arraySize, Value(0));
                        variables[varName] = arrVal;
                        // cerr << "DEBUG: Declared array " << varName << " with size " << entry->arraySize << endl;
                    } else {
                        // Initialize scalar to 0
                        variables[varName] = Value(0);
                    }
                    break;
                }
                entry = entry->next;
            }
        }
        varNode = varNode->rightSibling;
    }
}

void Interpreter::executeStatement(ASTNode* node) {
    if (!node) return;
    
    if (node->kind == "Assign") {
        executeAssignment(node);
    } else if (node->kind == "If") {
        executeIf(node);
    } else if (node->kind == "While") {
        executeWhile(node);
    } else if (node->kind == "For") {
        executeFor(node);
    } else if (node->kind == "Return") {
        executeReturn(node);
    } else if (node->kind == "Call") {
        executeCall(node);
    } else if (node->kind == "Printf") {
        executePrintf(node);
    }
}

void Interpreter::executeAssignment(ASTNode* node) {
    if (!node || node->kind != "Assign") return;
    
    ASTNode* lhs = node->leftChild;
    ASTNode* rhs = lhs ? lhs->rightSibling : nullptr;
    
    if (!lhs || !rhs) return;
    
    // Evaluate RHS
    Value val = evaluateExpression(rhs);
    
    // Assign to LHS
    if (lhs->kind == "Id") {
        // Check if it's a string being assigned to a char array
        if (val.type == Value::STRING && variables.find(lhs->text) != variables.end() 
            && variables[lhs->text].type == Value::ARRAY) {
            // String to char array assignment
            // cerr << "DEBUG: Assigning string '" << val.strVal << "' to array " << lhs->text << endl;
            for (size_t i = 0; i < val.strVal.length() && i < variables[lhs->text].arrayVal.size(); i++) {
                variables[lhs->text].arrayVal[i] = Value(val.strVal[i]);
                // cerr << "DEBUG: array[" << i << "] = '" << val.strVal[i] << "'" << endl;
            }
        } else {
            setVariable(lhs->text, val);
        }
    } else if (lhs->kind == "ArrAt") {
        // Array assignment: array[index] = value
        // Array name is in lhs->text, index is in lhs->leftChild
        string arrayName = lhs->text;
        ASTNode* indexExpr = lhs->leftChild;
        if (indexExpr) {
            Value indexVal = evaluateExpression(indexExpr);
            setArrayElement(arrayName, toInt(indexVal), val);
        }
    }
}

void Interpreter::executeIf(ASTNode* node) {
    if (!node || node->kind != "If") return;
    
    ASTNode* condition = node->leftChild;
    ASTNode* thenBlock = condition ? condition->rightSibling : nullptr;
    ASTNode* elseBlock = thenBlock ? thenBlock->rightSibling : nullptr;
    
    if (!condition) return;
    
    Value condVal = evaluateExpression(condition);
    
    if (toBool(condVal)) {
        // cerr << "DEBUG: If condition TRUE" << endl;
        if (thenBlock) {
            if (thenBlock->kind == "Block") {
                executeBlock(thenBlock);
            } else {
                executeStatement(thenBlock);
            }
        }
    } else {
        if (elseBlock && elseBlock->kind == "Else") {
            // The else content is the rightSibling of the Else node
            ASTNode* elseStmt = elseBlock->rightSibling;
            if (elseStmt) {
                if (elseStmt->kind == "Block") {
                    executeBlock(elseStmt);
                } else {
                    executeStatement(elseStmt);
                }
            }
        }
    }
}

void Interpreter::executeWhile(ASTNode* node) {
    if (!node || node->kind != "While") return;
    
    ASTNode* condition = node->leftChild;
    ASTNode* body = condition ? condition->rightSibling : nullptr;
    
    if (!condition) return;
    
    while (toBool(evaluateExpression(condition)) && !returnFlag) {
        if (body) {
            if (body->kind == "Block") {
                executeBlock(body);
            } else {
                executeStatement(body);
            }
        }
    }
}

void Interpreter::executeFor(ASTNode* node) {
    if (!node || node->kind != "For") return;
    
    ASTNode* init = node->leftChild;
    ASTNode* condition = init ? init->rightSibling : nullptr;
    ASTNode* update = condition ? condition->rightSibling : nullptr;
    ASTNode* body = update ? update->rightSibling : nullptr;
    
    // Execute initialization
    if (init) executeStatement(init);
    
    // Loop
    while (condition && !returnFlag) {
        Value condVal = evaluateExpression(condition);
        bool condBool = toBool(condVal);
        if (!condBool) break;
        if (body) {
            if (body->kind == "Block") {
                executeBlock(body);
            } else {
                executeStatement(body);
            }
        }
        if (update) executeStatement(update);
    }
}

void Interpreter::executeReturn(ASTNode* node) {
    if (!node || node->kind != "Return") return;
    
    ASTNode* expr = node->leftChild;
    if (expr) {
        returnValue = evaluateExpression(expr);
        // cerr << "DEBUG: Return value = " << toInt(returnValue) << endl;
    }
    returnFlag = true;
}

void Interpreter::executeCall(ASTNode* node) {
    if (!node || node->kind != "Call") return;
    
    string funcName = node->text;
    // cerr << "DEBUG: Calling function " << funcName << endl;
    
    // Collect arguments
    vector<Value> args;
    ASTNode* arg = node->leftChild;
    while (arg) {
        Value v = evaluateExpression(arg);
        // cerr << "DEBUG: Arg type=" << v.type << " intVal=" << toInt(v) << " charVal='" << toChar(v) << "'" << endl;
        args.push_back(v);
        arg = arg->rightSibling;
    }
    
    // Find and execute routine
    if (routines.find(funcName) != routines.end()) {
        bool savedReturnFlag = returnFlag;
        returnFlag = false;
        executeRoutine(routines[funcName], args);
        returnFlag = savedReturnFlag;
    }
}

void Interpreter::executePrintf(ASTNode* node) {
    if (!node || node->kind != "Printf") return;
    
    ASTNode* formatNode = node->leftChild;
    if (!formatNode || formatNode->kind != "Str") return;
    
    string format = formatNode->text;
    // cerr << "DEBUG: Printf format = '" << format << "'" << endl;
    
    // Collect arguments
    vector<Value> args;
    ASTNode* arg = formatNode->rightSibling;
    while (arg) {
        args.push_back(evaluateExpression(arg));
        arg = arg->rightSibling;
    }
    
    // Process format string
    size_t argIndex = 0;
    for (size_t i = 0; i < format.length(); i++) {
        if (format[i] == '\\' && i + 1 < format.length()) {
            // Handle escape sequences
            i++;
            if (format[i] == 'n') cout << '\n';
            else if (format[i] == 't') cout << '\t';
            else if (format[i] == '\\') cout << '\\';
            else if (format[i] == 'x') {
                // Hex escape \xHH
                if (i + 1 < format.length() && format[i+1] == '0') {
                    i++; // Skip the 0
                }
            }
            else cout << format[i];
        } else if (format[i] == '%' && i + 1 < format.length()) {
            // Handle format specifiers
            i++;
            if (format[i] == 'd' && argIndex < args.size()) {
                cout << toInt(args[argIndex++]);
            } else if (format[i] == 'c' && argIndex < args.size()) {
                cout << toChar(args[argIndex++]);
            } else if (format[i] == 's' && argIndex < args.size()) {
                cout << toString(args[argIndex++]);
            } else if (format[i] == '%') {
                cout << '%';
            }
        } else {
            cout << format[i];
        }
    }
}

Value Interpreter::evaluateExpression(ASTNode* node) {
    if (!node) return Value(0);
    

    // Handle different expression types
    if (node->kind == "Int") {
        return Value(stoi(node->text));
    } else if (node->kind == "Char") {
        // Handle character literals - text contains the actual character
        // The text might have the character value directly
        if (node->text.length() > 0) {
            // Find the actual character (skip any whitespace or quotes)
            size_t pos = 0;
            while (pos < node->text.length() && (node->text[pos] == ' ' || node->text[pos] == '\t')) {
                pos++;
            }
            if (pos >= node->text.length()) return Value('\0');
            
            char c = node->text[pos];
            // Handle escape sequences if needed
            if (c == '\\' && pos + 1 < node->text.length()) {
                if (node->text[pos+1] == 'n') c = '\n';
                else if (node->text[pos+1] == 't') c = '\t';
                else if (node->text[pos+1] == '0') c = '\0';
                else if (node->text[pos+1] == 'x') {
                    // Hex escape - parse the hex value
                    if (pos + 3 < node->text.length()) {
                        int val = 0;
                        for (size_t i = pos + 2; i < node->text.length() && i < pos + 4; i++) {
                            val = val * 16;
                            if (node->text[i] >= '0' && node->text[i] <= '9')
                                val += node->text[i] - '0';
                            else if (node->text[i] >= 'a' && node->text[i] <= 'f')
                                val += node->text[i] - 'a' + 10;
                            else if (node->text[i] >= 'A' && node->text[i] <= 'F')
                                val += node->text[i] - 'A' + 10;
                        }
                        c = (char)val;
                    }
                }
                else c = node->text[pos+1];
            }
            // cerr << "DEBUG: Char literal text='" << node->text << "' -> char='" << c << "' (int: " << (int)c << ")" << endl;
            return Value(c);
        }
        return Value('\0');
    } else if (node->kind == "Str") {
        // Process escape sequences in string
        string processed;
        for (size_t i = 0; i < node->text.length(); i++) {
            if (node->text[i] == '\\' && i + 1 < node->text.length()) {
                i++;
                if (node->text[i] == 'n') processed += '\n';
                else if (node->text[i] == 't') processed += '\t';
                else if (node->text[i] == '\\') processed += '\\';
                else if (node->text[i] == '"') processed += '"';
                else if (node->text[i] == 'x') {
                    // Hex escape \xHH
                    if (i + 1 < node->text.length()) {
                        int val = 0;
                        i++;
                        for (int j = 0; j < 2 && i < node->text.length(); j++, i++) {
                            val = val * 16;
                            if (node->text[i] >= '0' && node->text[i] <= '9')
                                val += node->text[i] - '0';
                            else if (node->text[i] >= 'a' && node->text[i] <= 'f')
                                val += node->text[i] - 'a' + 10;
                            else if (node->text[i] >= 'A' && node->text[i] <= 'F')
                                val += node->text[i] - 'A' + 10;
                            else {
                                i--;  // Not a hex digit, back up
                                break;
                            }
                        }
                        i--;  // Back up one since the loop will increment
                        processed += (char)val;
                    }
                }
                else processed += node->text[i];
            } else {
                processed += node->text[i];
            }
        }
        return Value(processed);
    } else if (node->kind == "Bool") {
        return Value(node->text == "true" || node->text == "1");
    } else if (node->kind == "Id") {
        Value v = getVariable(node->text);
        // if (node->text == "hex_digit") {
        //     cerr << "DEBUG: Reading hex_digit: type=" << v.type << " intVal=" << toInt(v) << " charVal='" << toChar(v) << "'" << endl;
        // }
        // If it's an array being used as a value (e.g., in printf), return it as-is
        return v;
    } else if (node->kind == "ArrAt") {
        // cerr << "DEBUG: ArrAt case entered" << endl;
        // Array access - array name is in node->text, index is in leftChild
        string arrayName = node->text;
        ASTNode* indexExpr = node->leftChild;
        // cerr << "DEBUG: arrayName=" << arrayName << " indexExpr=" << (indexExpr ? "exists" : "null") << endl;
        if (indexExpr) {
            Value indexVal = evaluateExpression(indexExpr);
            // cerr << "DEBUG: ArrAt evaluating " << arrayName << "[" << toInt(indexVal) << "]" << endl;
            return getArrayElement(arrayName, toInt(indexVal));
        }
        // cerr << "DEBUG: ArrAt returning 0 (no indexExpr)" << endl;
        return Value(0);
    } else if (node->kind == "Call") {
        // Function call
        string funcName = node->text;
        // cerr << "DEBUG: evaluateExpression Call to " << funcName << endl;
        vector<Value> args;
        ASTNode* arg = node->leftChild;
        while (arg) {
            // cerr << "DEBUG: Call arg node kind=" << arg->kind << " text=" << arg->text << endl;
            Value v = evaluateExpression(arg);
            // cerr << "DEBUG: Call arg value: type=" << v.type << " intVal=" << toInt(v) << " charVal='" << toChar(v) << "'" << endl;
            args.push_back(v);
            arg = arg->rightSibling;
        }
        
        if (routines.find(funcName) != routines.end()) {
            bool savedReturnFlag = returnFlag;
            Value savedReturnValue = returnValue;
            returnFlag = false;
            executeRoutine(routines[funcName], args);
            Value result = returnValue;
            // cerr << "DEBUG: Call returned: " << toInt(result) << endl;
            returnFlag = savedReturnFlag;
            returnValue = savedReturnValue;
            return result;
        }
        // cerr << "DEBUG: Function " << funcName << " not found!" << endl;
        return Value(0);
    } else if (node->kind == "Bin" || node->kind == "Un") {
        // Postfix expression
        return evaluatePostfix(node);
    }
    
    return Value(0);
}

Value Interpreter::evaluatePostfix(ASTNode* node) {
    if (!node) return Value(0);
    
    if (node->kind == "Bin") {
        // Binary expression: evaluate left and right, then apply operator
        ASTNode* left = node->leftChild;
        ASTNode* right = left ? left->rightSibling : nullptr;
        
        Value leftVal = evaluateExpression(left);
        Value rightVal = evaluateExpression(right);
        
        string op = node->text;
        int l = toInt(leftVal);
        int r = toInt(rightVal);
        
        if (op == "+") return Value(l + r);
        else if (op == "-") return Value(l - r);
        else if (op == "*") return Value(l * r);
        else if (op == "/") return Value(r != 0 ? l / r : 0);
        else if (op == "%") return Value(r != 0 ? l % r : 0);
        else if (op == "==") return Value(l == r);
        else if (op == "!=") return Value(l != r);
        else if (op == "<") return Value(l < r);
        else if (op == ">") return Value(l > r);
        else if (op == "<=") {
            bool result = l <= r;
            // cerr << "DEBUG: <= : " << l << " <= " << r << " = " << result << endl;
            return Value(result);
        }
        else if (op == ">=") {
            bool result = l >= r;
            // cerr << "DEBUG: >= : " << l << " >= " << r << " = " << result << endl;
            return Value(result);
        }
        else if (op == "&&") {
            bool result = toBool(leftVal) && toBool(rightVal);
            // cerr << "DEBUG: && : " << toInt(leftVal) << " && " << toInt(rightVal) << " = " << result << endl;
            return Value(result);
        }
        else if (op == "||") return Value(toBool(leftVal) || toBool(rightVal));
    } else if (node->kind == "Un") {
        // Unary expression: evaluate operand, then apply operator
        Value operand = evaluateExpression(node->leftChild);
        string op = node->text;
        
        if (op == "!") return Value(!toBool(operand));
        else if (op == "~") return Value(~toInt(operand));
        else if (op == "neg" || op == "-") return Value(-toInt(operand));
    }
    
    return Value(0);
}

void Interpreter::setVariable(const string& name, const Value& val) {
    variables[name] = val;
}

Value Interpreter::getVariable(const string& name) {
    if (variables.find(name) != variables.end()) {
        return variables[name];
    }
    return Value(0);
}

void Interpreter::setArrayElement(const string& name, int index, const Value& val) {
    if (variables.find(name) != variables.end() && variables[name].type == Value::ARRAY) {
        if (index >= 0 && index < (int)variables[name].arrayVal.size()) {
            variables[name].arrayVal[index] = val;
        }
    }
}

Value Interpreter::getArrayElement(const string& name, int index) {
    if (variables.find(name) != variables.end() && variables[name].type == Value::ARRAY) {
        if (index >= 0 && index < (int)variables[name].arrayVal.size()) {
            Value v = variables[name].arrayVal[index];
            // cerr << "DEBUG: getArrayElement(" << name << "[" << index << "]) = '" << toChar(v) << "' (int: " << toInt(v) << ")" << endl;
            return v;
        }
    }
    return Value(0);
}

int Interpreter::toInt(const Value& v) {
    if (v.type == Value::INT) return v.intVal;
    if (v.type == Value::CHAR) return (int)v.charVal;
    if (v.type == Value::BOOL) return v.boolVal ? 1 : 0;
    return 0;
}

char Interpreter::toChar(const Value& v) {
    if (v.type == Value::CHAR) return v.charVal;
    if (v.type == Value::INT) return (char)v.intVal;
    return '\0';
}

string Interpreter::toString(const Value& v) {
    if (v.type == Value::STRING) return v.strVal;
    if (v.type == Value::INT) return to_string(v.intVal);
    if (v.type == Value::CHAR) return string(1, v.charVal);
    if (v.type == Value::ARRAY) {
        // Convert char array to string
        string result;
        for (const Value& elem : v.arrayVal) {
            char c = toChar(elem);
            if (c == '\0') break;  // Stop at null terminator
            result += c;
        }
        return result;
    }
    return "";
}

bool Interpreter::toBool(const Value& v) {
    if (v.type == Value::BOOL) return v.boolVal;
    if (v.type == Value::INT) return v.intVal != 0;
    if (v.type == Value::CHAR) return v.charVal != 0;
    return false;
}
