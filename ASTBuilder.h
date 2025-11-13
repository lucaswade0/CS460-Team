#ifndef ASTBUILDER_H
#define ASTBUILDER_H

#include <string>
#include <ostream>
#include "CSTParser.h"

// Tiny LCRS AST limited to what the tests exercise.
struct ASTNode
{
    std::string kind; // Program, Routine, Block, Decl, Var, Assign, If, While, For, Return, Call, Printf, Bin, Un, Id, Int, Str, Char, Bool, ArrAt, Else
    std::string text; // identifier / literal / operator / callee
    int line;
    ASTNode *leftChild{}, *rightSibling{};
    ASTNode(std::string k = "", std::string t = "", int ln = 0)
        : kind(std::move(k)), text(std::move(t)), line(ln) {}
};

inline void ASTAddChild(ASTNode *p, ASTNode *c)
{
    if (!p || !c)
        return;
    if (!p->leftChild)
    {
        p->leftChild = c;
        return;
    }
    ASTNode *s = p->leftChild;
    while (s->rightSibling)
        s = s->rightSibling;
    s->rightSibling = c;
}

class ASTBuilder
{
public:
    static ASTNode *build(TreeNode *cstRoot);
    static void printExpected(ASTNode *root, std::ostream &out);
    static void free(ASTNode *root);

private:
    // Builders
    static ASTNode *buildProgram(TreeNode *n);
    static ASTNode *buildTopLevel(TreeNode *n); // function/procedure/global decl
    static ASTNode *buildRoutine(TreeNode *n);  // function or procedure
    static ASTNode *buildDecl(TreeNode *n);     // GlobalDecl or Declaration
    static ASTNode *buildBlock(TreeNode *n);
    static ASTNode *buildIf(TreeNode *n);
    static ASTNode *buildWhile(TreeNode *n);
    static ASTNode *buildFor(TreeNode *n);
    static ASTNode *buildReturn(TreeNode *n);
    static ASTNode *buildAssignment(TreeNode *n);
    static ASTNode *buildCall(TreeNode *n);

    // Statements & Expressions
    static ASTNode *buildStatement(TreeNode *n);

    // Expressions
    static ASTNode *buildExpr(TreeNode *n);
    static ASTNode *buildPrimary(TreeNode *n);
    static ASTNode *buildUnary(TreeNode *n);
    static ASTNode *buildBinary(TreeNode *n);
    static ASTNode *buildArrayAccess(TreeNode *n);

    // Printing
    enum class StrMode
    {
        Quoted,
        Bare
    };
    static void printBlock(ASTNode *n, std::ostream &out);
    static void printStmt(ASTNode *n, std::ostream &out, bool top = false);
    static void printRPN(ASTNode *n, std::ostream &out, StrMode mode = StrMode::Quoted);
    static void printCall(ASTNode *n, std::ostream &out);
    static void printPrintf(ASTNode *n, std::ostream &out);
    static void printAssignLHS(ASTNode *lhs, std::ostream &out); // moved inside class

    // Tiny CST helpers
    static TreeNode *skipTo(TreeNode *n, const std::string &value);
    static TreeNode *after(TreeNode *n, const std::string &value); // first node after token in same list
    static std::string takeString(TreeNode *stringLitCST);
    static std::string takeChar(TreeNode *charLitCST);
    static bool looksNumber(const std::string &s);
};

#endif
