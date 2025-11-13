#include "Tokenizer.h"
#include <cctype>
#include <iostream>

using namespace std;

bool Tokenizer::isHexDigit(char c) {
    if (c >= '0' && c <= '9') return true;
    if (c >= 'A' && c <= 'F') return true;
    if (c >= 'a' && c <= 'f') return true;
    return false;
}

bool Tokenizer::isLetterOrUnderscore(char c) {
    return isalpha(c) || c == '_';
}

bool Tokenizer::isLetterDigitOrUnderscore(char c) {
    return isalnum(c) || c == '_';
}

vector<Token> Tokenizer::tokenize(const string& input) {
    vector<Token> tokens;
    TokenizerState state = TOKENIZER_START;
    string currentToken = "";
    int line = 1;
    int column = 1;
    int tokenStartColumn = 1;
    bool inEscape = false;
    int hexDigitCount = 0;

    for (size_t i = 0; i <= input.length(); i++) {
        char c;
        if (i < input.length()) {
            c = input[i];
        } else {
            c = '\0';
        }

        switch (state) {
            case TOKENIZER_START:
                tokenStartColumn = column;
                currentToken = "";

                if (c == '\0') {
                    tokens.push_back({END_OF_FILE, "", line, column});
                    return tokens;
                }
                else if (c == ' ' || c == '\t') {
                    tokens.push_back({WHITESPACE, string(1, c), line, tokenStartColumn});
                }
                else if (c == '\n') {
                    tokens.push_back({NEWLINE, "\\n", line, tokenStartColumn});
                }
                else if (c == '(') tokens.push_back({L_PAREN, "(", line, tokenStartColumn});
                else if (c == ')') tokens.push_back({R_PAREN, ")", line, tokenStartColumn});
                else if (c == '[') tokens.push_back({L_BRACKET, "[", line, tokenStartColumn});
                else if (c == ']') tokens.push_back({R_BRACKET, "]", line, tokenStartColumn});
                else if (c == '{') tokens.push_back({L_BRACE, "{", line, tokenStartColumn});
                else if (c == '}') tokens.push_back({R_BRACE, "}", line, tokenStartColumn});
                else if (c == ';') tokens.push_back({SEMICOLON, ";", line, tokenStartColumn});
                else if (c == ',') tokens.push_back({COMMA, ",", line, tokenStartColumn});
                else if (c == '+') tokens.push_back({PLUS, "+", line, tokenStartColumn});
                else if (c == '-') {
                    if (i + 1 < input.length() && isdigit(input[i + 1])) {
                        currentToken = "-";
                        state = TOKENIZER_INTEGER;
                    } else {
                        tokens.push_back({MINUS, "-", line, tokenStartColumn});
                    }
                }
                else if (c == '*') tokens.push_back({ASTERISK, "*", line, tokenStartColumn});
                else if (c == '/') tokens.push_back({DIVIDE, "/", line, tokenStartColumn});
                else if (c == '%') tokens.push_back({MODULO, "%", line, tokenStartColumn});
                else if (c == '^') tokens.push_back({CARET, "^", line, tokenStartColumn});
                else if (c == '<') {
                    currentToken = "<";
                    state = TOKENIZER_LT;
                }
                else if (c == '>') {
                    currentToken = ">";
                    state = TOKENIZER_GT;
                }
                else if (c == '=') {
                    currentToken = "=";
                    state = TOKENIZER_EQUAL;
                }
                else if (c == '!') {
                    currentToken = "!";
                    state = TOKENIZER_NOT;
                }
                else if (c == '&') {
                    currentToken = "&";
                    state = TOKENIZER_AND;
                }
                else if (c == '|') {
                    currentToken = "|";
                    state = TOKENIZER_OR;
                }
                else if (c == '"') {
                    currentToken = "\"";
                    state = TOKENIZER_DOUBLE_STRING;
                }
                else if (c == '\'') {
                    currentToken = "'";
                    state = TOKENIZER_SINGLE_STRING;
                }
                else if (isdigit(c)) {
                    currentToken = c;
                    state = TOKENIZER_INTEGER;
                }
                else if (isLetterOrUnderscore(c)) {
                    currentToken = c;
                    state = TOKENIZER_IDENTIFIER;
                }
                else {
                    tokens.push_back({TOKEN_ERROR, string(1, c), line, tokenStartColumn});
                }
                break;

            case TOKENIZER_LT:
                if (c == '=') {
                    tokens.push_back({LT_EQUAL, "<=", line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else {
                    tokens.push_back({LT, "<", line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_GT:
                if (c == '=') {
                    tokens.push_back({GT_EQUAL, ">=", line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else {
                    tokens.push_back({GT, ">", line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_EQUAL:
                if (c == '=') {
                    tokens.push_back({BOOLEAN_EQUAL, "==", line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else {
                    tokens.push_back({ASSIGNMENT_OPERATOR, "=", line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_NOT:
                if (c == '=') {
                    tokens.push_back({BOOLEAN_NOT_EQUAL, "!=", line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else {
                    tokens.push_back({BOOLEAN_NOT, "!", line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_AND:
                if (c == '&') {
                    tokens.push_back({BOOLEAN_AND, "&&", line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else {
                    tokens.push_back({TOKEN_ERROR, "&", line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_OR:
                if (c == '|') {
                    tokens.push_back({BOOLEAN_OR, "||", line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else {
                    tokens.push_back({TOKEN_ERROR, "|", line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_INTEGER:
                if (isdigit(c)) {
                    currentToken += c;
                } else {
                    tokens.push_back({INTEGER, currentToken, line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_IDENTIFIER:
                if (isLetterDigitOrUnderscore(c)) {
                    currentToken += c;
                } else {
                    tokens.push_back({IDENTIFIER, currentToken, line, tokenStartColumn});
                    state = TOKENIZER_START;
                    i--;
                }
                break;

            case TOKENIZER_DOUBLE_STRING:
                currentToken += c;
                if (inEscape) {
                    if (c == 'x') {
                        state = TOKENIZER_HEX_ESCAPE;
                        hexDigitCount = 0;
                    }
                    inEscape = false;
                } else if (c == '\\') {
                    inEscape = true;
                } else if (c == '"') {
                    tokens.push_back({DOUBLE_QUOTED_STRING, currentToken, line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else if (c == '\0' || c == '\n') {
                    cerr << "Syntax error on line " << line << ": unterminated string quote." << endl;
                    exit(1);
                }
                break;

            case TOKENIZER_SINGLE_STRING:
                currentToken += c;
                if (inEscape) {
                    if (c == 'x') {
                        state = TOKENIZER_HEX_ESCAPE;
                        hexDigitCount = 0;
                    }
                    inEscape = false;
                } else if (c == '\\') {
                    inEscape = true;
                } else if (c == '\'') {
                    tokens.push_back({SINGLE_QUOTED_STRING, currentToken, line, tokenStartColumn});
                    state = TOKENIZER_START;
                } else if (c == '\0' || c == '\n') {
                    cerr << "Syntax error on line " << line << ": unterminated string quote." << endl;
                    exit(1);
                }
                break;

            case TOKENIZER_HEX_ESCAPE:
                currentToken += c;
                if (isHexDigit(c)) {
                    hexDigitCount++;
                    if (hexDigitCount == 2) {
                        if (currentToken.find('"') != string::npos) {
                            state = TOKENIZER_DOUBLE_STRING;
                        } else {
                            state = TOKENIZER_SINGLE_STRING;
                        }
                    }
                } else {
                    if (currentToken.find('"') != string::npos) {
                        state = TOKENIZER_DOUBLE_STRING;
                    } else {
                        state = TOKENIZER_SINGLE_STRING;
                    }
                    i--;
                }
                break;
        }

        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    return tokens;
}