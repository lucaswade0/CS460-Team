#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

using namespace std;

enum TokenType {
    L_PAREN, R_PAREN, L_BRACKET, R_BRACKET, L_BRACE, R_BRACE,
    SEMICOLON, COMMA, ASSIGNMENT_OPERATOR,
    PLUS, MINUS, ASTERISK, DIVIDE, MODULO, CARET,
    LT, GT, LT_EQUAL, GT_EQUAL,
    BOOLEAN_AND, BOOLEAN_OR, BOOLEAN_NOT, BOOLEAN_EQUAL, BOOLEAN_NOT_EQUAL,
    DOUBLE_QUOTED_STRING, SINGLE_QUOTED_STRING, DOUBLE_QUOTE, SINGLE_QUOTE, STRING,
    INTEGER, IDENTIFIER,
    WHITESPACE, NEWLINE,
    TOKEN_ERROR, END_OF_FILE
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
};

enum TokenizerState {
    TOKENIZER_START, TOKENIZER_IDENTIFIER, TOKENIZER_INTEGER,
    TOKENIZER_DOUBLE_STRING, TOKENIZER_SINGLE_STRING,
    TOKENIZER_LT, TOKENIZER_GT, TOKENIZER_EQUAL, TOKENIZER_NOT,
    TOKENIZER_AND, TOKENIZER_OR, TOKENIZER_ESCAPE, TOKENIZER_HEX_ESCAPE
};

class Tokenizer {
public:
    static vector<Token> tokenize(const string& input);

private:
    static bool isHexDigit(char c);
    static bool isLetterOrUnderscore(char c);
    static bool isLetterDigitOrUnderscore(char c);
};

#endif