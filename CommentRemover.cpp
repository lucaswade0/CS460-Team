#include "CommentRemover.h"
#include <sstream>
#include <iostream>

using namespace std;

string CommentRemover::removeComments(const string& input) {
    stringstream result;
    CommentState state = CODE;
    int line = 1;
    int blockStartLine = -1;

    for (size_t i = 0; i < input.length(); i++) {
        char ch = input[i];

        switch (state) {
            case CODE:
                if (ch == '*' && i + 1 < input.length() && input[i + 1] == '/') {
                    cerr << "ERROR: Program contains C-style, unterminated comment on line " << line << endl;
                    exit(1);
                }
                if (ch == '/') {
                    state = ONE_SLASH;
                } else if (ch == '"') {
                    state = IN_STRING;
                    result << '"';
                } else {
                    result << ch;
                }
                break;

            case ONE_SLASH:
                if (ch == '/') {
                    result << ' ' << ' ';
                    state = LINE_COMMENT;
                } else if (ch == '*') {
                    result << ' ' << ' ';
                    state = DOUBLE_COMMENT;
                    blockStartLine = line;
                } else {
                    result << '/' << ch;
                    state = CODE;
                }
                break;

            case LINE_COMMENT:
                if (ch == '\n') {
                    result << '\n';
                    state = CODE;
                    line++;
                } else {
                    result << ' ';
                }
                break;

            case DOUBLE_COMMENT:
                if (ch == '*') {
                    result << ' ';
                    state = END_DOUBLE_COMMENT;
                } else if (ch == '\n') {
                    result << '\n';
                } else {
                    result << ' ';
                }
                break;

            case END_DOUBLE_COMMENT:
                if (ch == '\n') {
                    result << '\n';
                } else {
                    result << ' ';
                }
                if (ch == '/') {
                    state = CODE;
                    blockStartLine = -1;
                } else if (ch != '*') {
                    state = DOUBLE_COMMENT;
                }
                break;

            case IN_STRING:
                result << ch;
                if (ch == '"') {
                    state = CODE;
                } else if (ch == '\\' && i + 1 < input.length()) {
                    result << input[i + 1];
                    i++;
                }
                break;
        }

        if (ch == '\n') {
            line++;
        }
    }

    if (state == DOUBLE_COMMENT || state == END_DOUBLE_COMMENT) {
        cerr << "ERROR: Program contains C-style, unterminated comment on line " << blockStartLine << endl;
        exit(1);
    }

    return result.str();
}