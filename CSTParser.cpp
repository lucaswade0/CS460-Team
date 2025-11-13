#include "CSTParser.h"
#include <iostream>
#include <set>

using namespace std;

TreeNode::TreeNode(string val, int ln) : value(val), line(ln), leftChild(nullptr), rightSibling(nullptr) {}

CSTParser::CSTParser(vector<Token> toks) : tokens(toks), current(0) {}


Token CSTParser::peek() {
    while (current < tokens.size() &&
           (tokens[current].type == WHITESPACE || tokens[current].type == NEWLINE)) {
        current++;
    }
    if (current < tokens.size()) {
        return tokens[current];
    }
    else {
        return Token{END_OF_FILE, "", 0, 0};
    }
}

Token CSTParser::advance() {
    Token tok = peek();
    if (current < tokens.size()) {
        current++;
    }
    return tok;
}

bool CSTParser::check(TokenType type) {
    return peek().type == type;
}

bool CSTParser::match(const string& value) {
    return peek().value == value;
}

Token CSTParser::expect(TokenType type, const string& errorMsg) {
    Token tok = peek();
    if (tok.type != type) {
        cerr << "Syntax error on line " << tok.line << ": " << errorMsg << endl;
        exit(1);
    }
    return advance();
}

Token CSTParser::expectValue(const string& value, const string& errorMsg) {
    Token tok = peek();
    if (tok.value != value) {
        cerr << "Syntax error on line " << tok.line << ": " << errorMsg << endl;
        exit(1);
    }
    return advance();
}

void CSTParser::addChild(TreeNode* parent, TreeNode* child) {
    if (!parent || !child) {
        return;
    }

    if (!parent->leftChild) {
        parent->leftChild = child;
    }
    else {
        TreeNode* sibling = parent->leftChild;
        while (sibling->rightSibling != nullptr) {
            sibling = sibling->rightSibling;
        }
        sibling->rightSibling = child;
    }
}

TreeNode* CSTParser::parseProgram() {
    TreeNode* root = new TreeNode("Program");

    while (!check(END_OF_FILE)) {
        Token nextToken = peek();

        if (match("function") || match("procedure")) {
            TreeNode* funcNode = parseFunctionOrProcedure();
            addChild(root, funcNode);
        }
        else if (match("int") || match("char") || match("bool") || match("void")) {
            TreeNode* globalVar = parseGlobalDeclaration();
            addChild(root, globalVar);
        } else {
            cerr << "Syntax error on line " << nextToken.line << ": unexpected token '" << nextToken.value << "'" << endl;
            exit(1);
        }
    }

    return root;
}

TreeNode* CSTParser::parseGlobalDeclaration() {
    TreeNode* node = new TreeNode("GlobalDecl");

    Token typeTok = advance();
    addChild(node, new TreeNode(typeTok.value, typeTok.line));

    do {
        if (check(COMMA)) {
            Token comma = advance();
            addChild(node, new TreeNode(comma.value, comma.line));
        }

        TreeNode* varNode = parseVariableDeclarator();
        addChild(node, varNode);

    } while (check(COMMA));

    Token semi = expect(SEMICOLON, "expected ';'");
    addChild(node, new TreeNode(semi.value, semi.line));

    return node;
}

TreeNode* CSTParser::parseFunctionOrProcedure() {
    Token keyword = advance();
    TreeNode* node = new TreeNode(keyword.value);
    addChild(node, new TreeNode(keyword.value, keyword.line));

    if (keyword.value == "function") {
        Token typeTok = advance();
        addChild(node, new TreeNode(typeTok.value, typeTok.line));
    }

    Token name = expect(IDENTIFIER, "expected identifier");

    if (isReservedWord(name.value)) {
        cerr << "Syntax error on line " << name.line << ": reserved word \"" << name.value
             << "\" cannot be used for the name of a function." << endl;
        exit(1);
    }

    addChild(node, new TreeNode(name.value, name.line));

    Token lparen = expect(L_PAREN, "expected '('");
    addChild(node, new TreeNode(lparen.value, lparen.line));

    addChild(node, parseParameters());

    Token rparen = expect(R_PAREN, "expected ')'");
    addChild(node, new TreeNode(rparen.value, rparen.line));

    addChild(node, parseBlock());

    return node;
}

TreeNode* CSTParser::parseParameters() {
    TreeNode* node = new TreeNode("Parameters");

    if (match("void")) {
        Token voidTok = advance();
        addChild(node, new TreeNode(voidTok.value, voidTok.line));
    } else {
        do {
            if (check(COMMA)) {
                Token comma = advance();
                addChild(node, new TreeNode(comma.value, comma.line));
            }

            TreeNode* param = parseParameter();
            addChild(node, param);

        } while (check(COMMA));
    }

    return node;
}

TreeNode* CSTParser::parseParameter() {
    TreeNode* node = new TreeNode("Parameter");

    Token typeTok = advance();
    addChild(node, new TreeNode(typeTok.value, typeTok.line));

    Token name = expect(IDENTIFIER, "expected parameter name");
    if (isReservedWord(name.value)) {
        cerr << "Syntax error on line " << name.line << ": reserved word \"" << name.value
             << "\" cannot be used for the name of a variable." << endl;
        exit(1);
    }

    addChild(node, new TreeNode(name.value, name.line));

    if (check(L_BRACKET)) {
        Token lbracket = advance();
        addChild(node, new TreeNode(lbracket.value, lbracket.line));

        if (!check(R_BRACKET)) {
            Token size = advance();
            addChild(node, new TreeNode(size.value, size.line));
        }

        Token rbracket = expect(R_BRACKET, "expected ']'");
        addChild(node, new TreeNode(rbracket.value, rbracket.line));
    }

    return node;
}

TreeNode* CSTParser::parseBlock() {
    TreeNode* node = new TreeNode("Block");

    Token lbrace = expect(L_BRACE, "expected '{'");
    addChild(node, new TreeNode(lbrace.value, lbrace.line));

    while (match("int") || match("char") || match("bool")) {
        addChild(node, parseDeclaration());
    }

    while (!check(R_BRACE) && !check(END_OF_FILE)) {
        addChild(node, parseStatement());
    }

    Token rbrace = expect(R_BRACE, "expected '}'");
    addChild(node, new TreeNode(rbrace.value, rbrace.line));

    return node;
}

TreeNode* CSTParser::parseDeclaration() {
    TreeNode* node = new TreeNode("Declaration");

    Token typeTok = advance();
    addChild(node, new TreeNode(typeTok.value, typeTok.line));

    do {
        if (check(COMMA)) {
            Token comma = advance();
            addChild(node, new TreeNode(comma.value, comma.line));
        }

        addChild(node, parseVariableDeclarator());

    } while (check(COMMA));

    Token semi = expect(SEMICOLON, "expected ';'");
    addChild(node, new TreeNode(semi.value, semi.line));

    return node;
}

TreeNode* CSTParser::parseVariableDeclarator() {
    TreeNode* node = new TreeNode("VarDecl");

    Token name = expect(IDENTIFIER, "expected identifier");

    if (isReservedWord(name.value)) {
        cerr << "Syntax error on line " << name.line << ": reserved word \"" << name.value
             << "\" cannot be used for the name of a variable." << endl;
        exit(1);
    }
    addChild(node, new TreeNode(name.value, name.line));

    if (check(L_BRACKET)) {
        Token lbracket = advance();
        addChild(node, new TreeNode(lbracket.value, lbracket.line));

        Token size = advance();

        if (size.type == INTEGER) {
            int sizeVal = stoi(size.value);
            if (sizeVal <= 0) {
                cerr << "Syntax error on line " << size.line << ": array declaration size must be a positive integer." << endl;
                exit(1);
            }
        }
        else if (size.type == MINUS) {
            cerr << "Syntax error on line " << size.line << ": array declaration size must be a positive integer." << endl;
            exit(1);
        }

        addChild(node, new TreeNode(size.value, size.line));

        Token rbracket = expect(R_BRACKET, "expected ']'");
        addChild(node, new TreeNode(rbracket.value, rbracket.line));
    }

    return node;
}

TreeNode* CSTParser::parseStatement() {
    if (match("if")) {
        return parseIfStatement();
    } else if (match("while")) {
        return parseWhileStatement();
    } else if (match("for")) {
        return parseForStatement();
    } else if (match("return")) {
        return parseReturnStatement();
    } else if (check(L_BRACE)) {
        return parseBlock();
    } else if (check(IDENTIFIER)) {
        return parseExpressionStatement();
    } else {
        Token tok = peek();
        cerr << "Syntax error on line " << tok.line << ": unexpected token '" << tok.value << "'" << endl;
        exit(1);
    }
}

TreeNode* CSTParser::parseIfStatement() {
    TreeNode* node = new TreeNode("IfStmt");

    Token ifTok = advance();
    addChild(node, new TreeNode(ifTok.value, ifTok.line));

    Token lparen = expect(L_PAREN, "expected '('");
    addChild(node, new TreeNode(lparen.value, lparen.line));

    addChild(node, parseExpression());

    Token rparen = expect(R_PAREN, "expected ')'");
    addChild(node, new TreeNode(rparen.value, rparen.line));

    addChild(node, parseStatement());

    if (match("else")) {
        Token elseTok = advance();
        addChild(node, new TreeNode(elseTok.value, elseTok.line));
        addChild(node, parseStatement());
    }

    return node;
}

TreeNode* CSTParser::parseWhileStatement() {
    TreeNode* node = new TreeNode("WhileStmt");

    Token whileTok = advance();
    addChild(node, new TreeNode(whileTok.value, whileTok.line));

    Token lparen = expect(L_PAREN, "expected '('");
    addChild(node, new TreeNode(lparen.value, lparen.line));

    addChild(node, parseExpression());

    Token rparen = expect(R_PAREN, "expected ')'");
    addChild(node, new TreeNode(rparen.value, rparen.line));

    addChild(node, parseStatement());

    return node;
}

TreeNode* CSTParser::parseForStatement() {
    TreeNode* node = new TreeNode("ForStmt");

    Token forTok = advance();
    addChild(node, new TreeNode(forTok.value, forTok.line));

    Token lparen = expect(L_PAREN, "expected '('");
    addChild(node, new TreeNode(lparen.value, lparen.line));

    addChild(node, parseAssignment());

    Token semi1 = expect(SEMICOLON, "expected ';'");
    addChild(node, new TreeNode(semi1.value, semi1.line));

    addChild(node, parseExpression());

    Token semi2 = expect(SEMICOLON, "expected ';'");
    addChild(node, new TreeNode(semi2.value, semi2.line));

    addChild(node, parseAssignment());

    Token rparen = expect(R_PAREN, "expected ')'");
    addChild(node, new TreeNode(rparen.value, rparen.line));

    addChild(node, parseStatement());

    return node;
}

TreeNode* CSTParser::parseReturnStatement() {
    TreeNode* node = new TreeNode("ReturnStmt");

    Token retTok = advance();
    addChild(node, new TreeNode(retTok.value, retTok.line));

    addChild(node, parseExpression());

    Token semi = expect(SEMICOLON, "expected ';'");
    addChild(node, new TreeNode(semi.value, semi.line));

    return node;
}

TreeNode* CSTParser::parseExpressionStatement() {
    Token name = peek();

    size_t saved = current;
    advance();
    Token lookahead = peek();
    current = saved;

    if (lookahead.type == ASSIGNMENT_OPERATOR || lookahead.type == L_BRACKET) {
        TreeNode* node = parseAssignment();

        Token semi = expect(SEMICOLON, "expected ';'");
        addChild(node, new TreeNode(semi.value, semi.line));

        return node;

    } else if (lookahead.type == L_PAREN) {
        TreeNode* node = parseFunctionCall();
        Token semi = expect(SEMICOLON, "expected ';'");
        TreeNode* wrapper = new TreeNode("ExprStmt");

        addChild(wrapper, node);
        addChild(wrapper, new TreeNode(semi.value, semi.line));

        return wrapper;
    } else {
        cerr << "Syntax error on line " << name.line << ": unexpected token" << endl;
        exit(1);
    }
}

TreeNode* CSTParser::parseAssignment() {
    TreeNode* node = new TreeNode("Assignment");

    Token name = expect(IDENTIFIER, "expected identifier");
    addChild(node, new TreeNode(name.value, name.line));

    if (check(L_BRACKET)) {
        Token lbracket = advance();
        addChild(node, new TreeNode(lbracket.value, lbracket.line));

        addChild(node, parseExpression());

        Token rbracket = expect(R_BRACKET, "expected ']'");
        addChild(node, new TreeNode(rbracket.value, rbracket.line));
    }

    Token eq = expect(ASSIGNMENT_OPERATOR, "expected '='");
    addChild(node, new TreeNode(eq.value, eq.line));

    addChild(node, parseExpression());

    return node;
}

TreeNode* CSTParser::parseExpression() {
    return parseLogicalOr();
}

TreeNode* CSTParser::parseLogicalOr() {
    TreeNode* left = parseLogicalAnd();

    while (check(BOOLEAN_OR)) {
        Token op = advance();
        TreeNode* binOpNode = new TreeNode("BinaryOp");
        addChild(binOpNode, left);
        addChild(binOpNode, new TreeNode(op.value, op.line));
        TreeNode* right = parseLogicalAnd();
        addChild(binOpNode, right);
        left = binOpNode;
    }

    return left;
}

TreeNode* CSTParser::parseLogicalAnd() {
    TreeNode* left = parseEquality();

    while (check(BOOLEAN_AND)) {
        Token op = advance();
        TreeNode* node = new TreeNode("BinaryOp");
        addChild(node, left);
        addChild(node, new TreeNode(op.value, op.line));
        addChild(node, parseEquality());
        left = node;
    }

    return left;
}

TreeNode* CSTParser::parseEquality() {
    TreeNode* left = parseRelational();

    while (check(BOOLEAN_EQUAL) || check(BOOLEAN_NOT_EQUAL)) {
        Token op = advance();
        TreeNode* node = new TreeNode("BinaryOp");
        addChild(node, left);
        addChild(node, new TreeNode(op.value, op.line));
        addChild(node, parseRelational());
        left = node;
    }

    return left;
}

TreeNode* CSTParser::parseRelational() {
    TreeNode* left = parseAdditive();

    while (check(LT) || check(GT) || check(LT_EQUAL) || check(GT_EQUAL)) {
        Token op = advance();
        TreeNode* node = new TreeNode("BinaryOp");
        addChild(node, left);
        addChild(node, new TreeNode(op.value, op.line));
        addChild(node, parseAdditive());
        left = node;
    }

    return left;
}

TreeNode* CSTParser::parseAdditive() {
    TreeNode* left = parseMultiplicative();

    while (check(PLUS) || check(MINUS)) {
        Token op = advance();
        TreeNode* node = new TreeNode("BinaryOp");
        addChild(node, left);
        addChild(node, new TreeNode(op.value, op.line));
        addChild(node, parseMultiplicative());
        left = node;
    }

    return left;
}

TreeNode* CSTParser::parseMultiplicative() {
    TreeNode* left = parseUnary();

    while (check(ASTERISK) || check(DIVIDE) || check(MODULO)) {
        Token op = advance();
        TreeNode* node = new TreeNode("BinaryOp");
        addChild(node, left);
        addChild(node, new TreeNode(op.value, op.line));
        addChild(node, parseUnary());
        left = node;
    }

    return left;
}

TreeNode* CSTParser::parseUnary() {
    if (check(BOOLEAN_NOT) || check(MINUS)) {
        Token op = advance();
        TreeNode* node = new TreeNode("UnaryOp");
        addChild(node, new TreeNode(op.value, op.line));
        addChild(node, parseUnary());
        return node;
    }

    return parsePrimary();
}

TreeNode* CSTParser::parsePrimary() {
    if (check(INTEGER)) {
        Token num = advance();
        return new TreeNode(num.value, num.line);

    } else if (check(IDENTIFIER)) {
        size_t saved = current;
        Token name = advance();
        Token next = peek();
        current = saved;

        if (next.type == L_PAREN) {
            return parseFunctionCall();
        } else if (next.type == L_BRACKET) {
            advance();
            TreeNode* node = new TreeNode("ArrayAccess");
            addChild(node, new TreeNode(name.value, name.line));

            Token lbracket = advance();
            addChild(node, new TreeNode(lbracket.value, lbracket.line));

            addChild(node, parseExpression());

            Token rbracket = expect(R_BRACKET, "expected ']'");
            addChild(node, new TreeNode(rbracket.value, rbracket.line));

            return node;
        } else {
            advance();
            return new TreeNode(name.value, name.line);
        }
    } else if (check(SINGLE_QUOTED_STRING)) {
        Token str = advance();
        TreeNode* node = new TreeNode("CharLiteral");
        string content = str.value.substr(1, str.value.length() - 2);
        addChild(node, new TreeNode("'", str.line));
        addChild(node, new TreeNode(content, str.line));
        addChild(node, new TreeNode("'", str.line));
        return node;

    } else if (check(DOUBLE_QUOTED_STRING)) {
        Token str = advance();
        TreeNode* node = new TreeNode("StringLiteral");
        string content = str.value.substr(1, str.value.length() - 2);
        addChild(node, new TreeNode("\"", str.line));
        addChild(node, new TreeNode(content, str.line));
        addChild(node, new TreeNode("\"", str.line));
        return node;

    } else if (check(L_PAREN)) {
        Token lparen = advance();
        TreeNode* node = new TreeNode("ParenExpr");
        addChild(node, new TreeNode(lparen.value, lparen.line));
        addChild(node, parseExpression());
        Token rparen = expect(R_PAREN, "expected ')'");
        addChild(node, new TreeNode(rparen.value, rparen.line));
        return node;
    } else {
        Token tok = peek();
        cerr << "Syntax error on line " << tok.line << ": unexpected token '" << tok.value << "'" << endl;
        exit(1);
    }
}

TreeNode* CSTParser::parseFunctionCall() {
    TreeNode* node = new TreeNode("FunctionCall");

    Token name = expect(IDENTIFIER, "expected function name");
    addChild(node, new TreeNode(name.value, name.line));

    Token lparen = expect(L_PAREN, "expected '('");
    addChild(node, new TreeNode(lparen.value, lparen.line));

    if (!check(R_PAREN)) {
        do {
            if (check(COMMA)) {
                Token comma = advance();
                addChild(node, new TreeNode(comma.value, comma.line));
            }
            addChild(node, parseExpression());
        }
        while (check(COMMA));
    }

    Token rparen = expect(R_PAREN, "expected ')'");
    addChild(node, new TreeNode(rparen.value, rparen.line));

    return node;
}

TreeNode* CSTParser::parse() {
    return parseProgram();
}

void CSTParser::printTree(TreeNode* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) cout << "  ";
    cout << node->value << endl;
    printTree(node->leftChild, depth + 1);
    printTree(node->rightSibling, depth);
}

void CSTParser::printCST(vector<Token>& tokens, ofstream& out) {
    int currentLine = 0;
    bool firstOnLine = true;

    for (const Token& tok : tokens) {
        if (tok.type == WHITESPACE || tok.type == END_OF_FILE) {
            continue;
        }

        if (tok.type == NEWLINE) {
            if (!firstOnLine) {
                out << endl;
                currentLine++;
                firstOnLine = true;
            }
            continue;
        }

        if (tok.type == DOUBLE_QUOTED_STRING) {
            if (tok.line != currentLine) {
                if (!firstOnLine) {
                    out << endl;
                }
                currentLine = tok.line;
                firstOnLine = true;
            }

            if (!firstOnLine) {
                out << "   ";
            }
            string content = tok.value.substr(1, tok.value.length() - 2);
            out << "\"   " << content << "   \"";
            firstOnLine = false;

        } else if (tok.type == SINGLE_QUOTED_STRING) {
            if (tok.line != currentLine) {
                if (!firstOnLine) out << endl;
                currentLine = tok.line;
                firstOnLine = true;
            }

            if (!firstOnLine) {
                out << "   ";
            }
            string content = tok.value.substr(1, tok.value.length() - 2);
            out << "'   " << content << "   '";
            firstOnLine = false;
        } else {
            if (tok.line != currentLine) {
                if (!firstOnLine) out << endl;
                currentLine = tok.line;
                firstOnLine = true;
            }

            if (!firstOnLine) {
                out << "   ";
            }
            out << tok.value;
            firstOnLine = false;
        }
    }

    out << endl;
}

bool CSTParser::isReservedWord(const string& word) {
    const set<string> reserved = {
        "int", "char", "void", "bool",
        "function", "procedure", "if", "else", "while", "for", "return",
        "printf", "TRUE", "FALSE"
    };

    return reserved.find(word) != reserved.end();
}
