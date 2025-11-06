#ifndef COMMENTREMOVER_H
#define COMMENTREMOVER_H

#include <string>

using namespace std;

enum CommentState {
    CODE, ONE_SLASH, LINE_COMMENT, DOUBLE_COMMENT, END_DOUBLE_COMMENT, IN_STRING
};

class CommentRemover {
public:
    static string removeComments(const string& input);
};

#endif