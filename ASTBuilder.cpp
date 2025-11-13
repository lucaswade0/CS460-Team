#include "ASTBuilder.h"
#include <cctype>

// ---------------- CST helpers ----------------
TreeNode *ASTBuilder::skipTo(TreeNode *n, const std::string &value)
{
    for (; n && n->value != value; n = n->rightSibling)
    {
    }
    return n;
}
TreeNode *ASTBuilder::after(TreeNode *n, const std::string &value)
{
    n = skipTo(n, value);
    return n ? n->rightSibling : nullptr;
}
std::string ASTBuilder::takeString(TreeNode *n)
{
    // StringLiteral -> '"' content '"'
    std::string val = "";
    if (n && n->leftChild && n->leftChild->rightSibling) {
        val = n->leftChild->rightSibling->value;
    }
    //remove trailing qiuote if its there
    if (!val.empty() && val.back() == '"') {
        val.pop_back();
    }

    return val;
}
std::string ASTBuilder::takeChar(TreeNode *n)
{
    // CharLiteral -> '\'' content '\''
    std::string val = "";
    if (n && n->leftChild && n->leftChild->rightSibling) {
        val = n->leftChild->rightSibling->value;
    }
    //Remove trailing quote if there
    if (!val.empty() && val.back() == '\'') {
        val.pop_back();
    }

    return val;
}
bool ASTBuilder::looksNumber(const std::string &s)
{
    if (s.empty())
        return false;
    size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    if (i >= s.size())
        return false;
    for (; i < s.size(); ++i)
        if (!std::isdigit((unsigned char)s[i]))
            return false;
    return true;
}

// ---------------- Public ----------------
ASTNode *ASTBuilder::build(TreeNode *cstRoot) { return cstRoot ? buildProgram(cstRoot) : nullptr; }

void ASTBuilder::printExpected(ASTNode *root, std::ostream &out)
{
    if (!root) {
        return;
    }
    for (ASTNode *c = root->leftChild; c; c = c->rightSibling) {
        printStmt(c, out, true);
    }
    out << "\n";
}

void ASTBuilder::free(ASTNode *root)
{
    if (!root)
        return;
    free(root->leftChild);
    free(root->rightSibling);
    delete root;
}

// ---------------- Builders ----------------
ASTNode *ASTBuilder::buildProgram(TreeNode *n)
{
    ASTNode *prog = new ASTNode("Program", "", n->line);
    for (TreeNode *c = n->leftChild; c; c = c->rightSibling)
        if (ASTNode *t = buildTopLevel(c))
            ASTAddChild(prog, t);
    return prog;
}

ASTNode *ASTBuilder::buildTopLevel(TreeNode *n)
{
    if (!n)
        return nullptr;
    if (n->value == "function" || n->value == "procedure")
        return buildRoutine(n);
    if (n->value == "GlobalDecl")
        return buildDecl(n);
    return nullptr;
}

ASTNode *ASTBuilder::buildRoutine(TreeNode *n)
{
    ASTNode *r = new ASTNode("Routine", "", n->line);
    if (TreeNode *blk = skipTo(n->leftChild, "Block"))
        ASTAddChild(r, buildBlock(blk));
    return r;
}

ASTNode *ASTBuilder::buildDecl(TreeNode *n)
{
    ASTNode *d = new ASTNode("Decl", "", n->line);
    TreeNode *type = n->leftChild;
    for (TreeNode *c = type ? type->rightSibling : nullptr; c; c = c->rightSibling)
    {
        if (c->value == "VarDecl")
        {
            TreeNode *name = c->leftChild;
            ASTAddChild(d, new ASTNode("Var", name ? name->value : "", name ? name->line : n->line));
        }
    }
    return d;
}

ASTNode *ASTBuilder::buildStatement(TreeNode *n)
{
    if (!n)
        return nullptr;
    if (n->value == "IfStmt")
        return buildIf(n);
    if (n->value == "WhileStmt")
        return buildWhile(n);
    if (n->value == "ForStmt")
        return buildFor(n);
    if (n->value == "ReturnStmt")
        return buildReturn(n);
    if (n->value == "Assignment")
        return buildAssignment(n);
    if (n->value == "FunctionCall")
        return buildCall(n);
    if (n->value == "Block")
        return buildBlock(n);
    if (n->value == "Declaration")
        return buildDecl(n);
    if (n->value == "ExprStmt")
    {
        TreeNode *call = n->leftChild;
        return (call && call->value == "FunctionCall") ? buildCall(call) : nullptr;
    }
    return nullptr;
}

ASTNode *ASTBuilder::buildBlock(TreeNode *n)
{
    ASTNode *b = new ASTNode("Block", "", n->line);
    for (TreeNode *c = n->leftChild; c; c = c->rightSibling)
        if (ASTNode *s = buildStatement(c))
            ASTAddChild(b, s);
    return b;
}

ASTNode *ASTBuilder::buildIf(TreeNode *n)
{
    ASTNode *node = new ASTNode("If", "", n->line);
    if (TreeNode *cond = after(n->leftChild, "("))
        ASTAddChild(node, buildExpr(cond));
    if (TreeNode *thenS = after(n->leftChild, ")"))
        ASTAddChild(node, buildStatement(thenS));
    if (TreeNode *e = skipTo(n->leftChild, "else"))
    {
        ASTAddChild(node, new ASTNode("Else", "", e->line));
        ASTAddChild(node, buildStatement(e->rightSibling));
    }
    return node;
}

ASTNode *ASTBuilder::buildWhile(TreeNode *n)
{
    ASTNode *node = new ASTNode("While", "", n->line);
    if (TreeNode *cond = after(n->leftChild, "("))
        ASTAddChild(node, buildExpr(cond));
    if (TreeNode *body = after(n->leftChild, ")"))
        ASTAddChild(node, buildStatement(body));
    return node;
}

ASTNode *ASTBuilder::buildFor(TreeNode *n)
{
    ASTNode *node = new ASTNode("For", "", n->line);
    TreeNode *cur = after(n->leftChild, "(");

    if (cur && cur->value == "Assignment")
    {
        ASTAddChild(node, buildAssignment(cur));
        cur = after(cur, ";");
    }
    if (cur)
    {
        ASTAddChild(node, buildExpr(cur));
        cur = after(cur, ";");
    }
    if (cur && cur->value == "Assignment")
        ASTAddChild(node, buildAssignment(cur));

    if (TreeNode *body = after(n->leftChild, ")"))
        ASTAddChild(node, buildStatement(body));
    return node;
}

ASTNode *ASTBuilder::buildReturn(TreeNode *n)
{
    ASTNode *r = new ASTNode("Return", "", n->line);
    if (TreeNode *expr = after(n->leftChild, "return"))
        ASTAddChild(r, buildExpr(expr));
    return r;
}

ASTNode *ASTBuilder::buildAssignment(TreeNode *n)
{
    ASTNode *as = new ASTNode("Assign", "", n->line);
    TreeNode *lhs = n->leftChild;
    ASTNode *L = nullptr;
    if (lhs && lhs->rightSibling && lhs->rightSibling->value == "[")
    {
        L = new ASTNode("ArrAt", lhs->value, lhs->line);
        ASTAddChild(L, buildExpr(lhs->rightSibling->rightSibling));
    }
    else if (lhs)
    {
        L = new ASTNode("Id", lhs->value, lhs->line);
    }
    if (L)
        ASTAddChild(as, L);
    if (TreeNode *rhs = after(n->leftChild, "="))
        ASTAddChild(as, buildExpr(rhs));
    return as;
}

ASTNode *ASTBuilder::buildCall(TreeNode *n)
{
    TreeNode *name = n->leftChild;
    std::string who = name ? name->value : "";
    ASTNode *call = new ASTNode(who == "printf" ? "Printf" : "Call", who, name ? name->line : n->line);
    TreeNode *a = after(name ? name->rightSibling : nullptr, "(");
    for (; a && a->value != ")"; a = a->rightSibling)
    {
        if (a->value == ",")
            continue;
        ASTAddChild(call, buildExpr(a));
        while (a->rightSibling && a->rightSibling->value != "," && a->rightSibling->value != ")")
            a = a->rightSibling;
    }
    return call;
}

// ---------------- Expressions ----------------
ASTNode *ASTBuilder::buildExpr(TreeNode *n)
{
    if (!n)
        return nullptr;
    if (n->value == "BinaryOp")
        return buildBinary(n);
    if (n->value == "UnaryOp")
        return buildUnary(n);
    if (n->value == "ParenExpr")
    {
        TreeNode *i = n->leftChild ? n->leftChild->rightSibling : nullptr;
        return buildExpr(i);
    }
    if (n->value == "FunctionCall")
        return buildCall(n);
    if (n->value == "ArrayAccess")
        return buildArrayAccess(n);
    return buildPrimary(n);
}
ASTNode *ASTBuilder::buildPrimary(TreeNode *n)
{
    if (!n)
        return nullptr;
    if (looksNumber(n->value))
        return new ASTNode("Int", n->value, n->line);
    if (n->value == "TRUE" || n->value == "FALSE")
        return new ASTNode("Bool", n->value, n->line);
    if (n->value == "StringLiteral")
        return new ASTNode("Str", takeString(n), n->line);
    if (n->value == "CharLiteral")
        return new ASTNode("Char", takeChar(n), n->line);
    // identifier: anything that isn't punctuation/keywords we see around primaries
    if (n->value != "(" && n->value != ")" && n->value != "[" && n->value != "]" && n->value != "{" && n->value != "}" &&
        n->value != "Parameters" && n->value != "Parameter" && n->value != "," && n->value != ";" && n->value != "=")
        return new ASTNode("Id", n->value, n->line);
    return new ASTNode("Id", "", n->line);
}
ASTNode *ASTBuilder::buildUnary(TreeNode *n)
{
    TreeNode *op = n->leftChild;
    ASTNode *u = new ASTNode("Un", op ? op->value : "", n->line);
    ASTAddChild(u, buildExpr(op ? op->rightSibling : nullptr));
    return u;
}
ASTNode *ASTBuilder::buildBinary(TreeNode *n)
{
    TreeNode *L = n->leftChild, *op = L ? L->rightSibling : nullptr;
    ASTNode *b = new ASTNode("Bin", op ? op->value : "", n->line);
    ASTAddChild(b, buildExpr(L));
    ASTAddChild(b, buildExpr(op ? op->rightSibling : nullptr));
    return b;
}
ASTNode *ASTBuilder::buildArrayAccess(TreeNode *n)
{
    TreeNode *name = n->leftChild, *idx = name && name->rightSibling ? name->rightSibling->rightSibling : nullptr;
    ASTNode *arr = new ASTNode("ArrAt", name ? name->value : "", name ? name->line : n->line);
    ASTAddChild(arr, buildExpr(idx));
    return arr;
}

// ---------------- Printing ----------------
void ASTBuilder::printAssignLHS(ASTNode *lhs, std::ostream &out)
{
    if (!lhs)
        return;
    if (lhs->kind == "ArrAt")
    {
        out << lhs->text << "   [   ";
        ASTBuilder::printRPN(lhs->leftChild, out);
        out << "   ]   ";
    }
    else
    {
        out << lhs->text << "   ";
    }
}

void ASTBuilder::printBlock(ASTNode *n, std::ostream &out)
{
    out << "BEGIN BLOCK\n";
    for (ASTNode *c = n->leftChild; c; c = c->rightSibling)
        printStmt(c, out);
    out << "END BLOCK\n";
}

void ASTBuilder::printStmt(ASTNode *n, std::ostream &out, bool top)
{
    if (!n)
        return;

    if (n->kind == "Decl")
    {
        int cnt = 0;
        for (ASTNode *v = n->leftChild; v; v = v->rightSibling)
            if (v->kind == "Var")
            {
                ++cnt;
                out << "DECLARATION\n";
            }
        if (cnt == 0)
            out << "DECLARATION\n";
        return;
    }
    if (n->kind == "Block")
    {
        printBlock(n, out);
        return;
    }

    if (n->kind == "Routine")
    {
        if (top)
            out << "DECLARATION\n";
        for (ASTNode *c = n->leftChild; c; c = c->rightSibling)
            if (c->kind == "Block")
                printBlock(c, out);
        return;
    }

    if (n->kind == "Assign")
    {
        out << "ASSIGNMENT   ";
        ASTNode *lhs = n->leftChild, *rhs = lhs ? lhs->rightSibling : nullptr;
        printAssignLHS(lhs, out);
        if (rhs)
        {
            printRPN(rhs, out);
            out << "   =\n";
        }
        else
            out << "=\n";
        return;
    }

    if (n->kind == "If")
    {
        out << "IF   ";
        ASTNode *cond = n->leftChild, *thenS = cond ? cond->rightSibling : nullptr, *maybe = thenS ? thenS->rightSibling : nullptr;
        printRPN(cond, out);
        out << "\n";
        printStmt(thenS, out);
        if (maybe && maybe->kind == "Else")
        {
            out << "ELSE\n";
            printStmt(maybe->rightSibling, out);
        }
        return;
    }

    if (n->kind == "While")
    {
        out << "WHILE   ";
        ASTNode *cond = n->leftChild, *body = cond ? cond->rightSibling : nullptr;
        printRPN(cond, out);
        out << "\n";
        printStmt(body, out);
        return;
    }

    if (n->kind == "For")
    {
        ASTNode *init = n->leftChild, *cond = init ? init->rightSibling : nullptr, *step = cond ? cond->rightSibling : nullptr, *body = step ? step->rightSibling : nullptr;
        out << "FOR EXPRESSION 1   ";
        {
            ASTNode *L = init ? init->leftChild : nullptr, *R = L ? L->rightSibling : nullptr;
            printAssignLHS(L, out);
            if (R)
            {
                printRPN(R, out);
                out << "   =\n";
            }
            else
                out << "=\n";
        }
        out << "FOR EXPRESSION 2   ";
        printRPN(cond, out);
        out << "\n";
        out << "FOR EXPRESSION 3   ";
        {
            ASTNode *L = step ? step->leftChild : nullptr, *R = L ? L->rightSibling : nullptr;
            printAssignLHS(L, out);
            if (R)
            {
                printRPN(R, out);
                out << "   =\n";
            }
            else
                out << "=\n";
        }
        printStmt(body, out);
        return;
    }

    if (n->kind == "Return")
    {
        out << "RETURN   ";
        printRPN(n->leftChild, out);
        out << "\n";
        return;
    }
    if (n->kind == "Call")
    {
        printCall(n, out);
        return;
    }
    if (n->kind == "Printf")
    {
        printPrintf(n, out);
        return;
    }

    for (ASTNode *c = n->leftChild; c; c = c->rightSibling)
        printStmt(c, out);
}

void ASTBuilder::printCall(ASTNode *n, std::ostream &out)
{
    out << "CALL   " << n->text << "   (   ";
    bool first = true;
    for (ASTNode *a = n->leftChild; a; a = a->rightSibling)
    {
        if (!first)
            out << "   ,   ";
        printRPN(a, out);
        first = false;
    }
    out << "   )\n";
}
void ASTBuilder::printPrintf(ASTNode *n, std::ostream &out)
{
    out << "PRINTF   ";
    ASTNode *fmt = n->leftChild;
    if (fmt)
    {
        printRPN(fmt, out, StrMode::Bare);
        for (ASTNode *a = fmt->rightSibling; a; a = a->rightSibling)
        {
            out << "   ";
            printRPN(a, out);
        }
    }
    out << "\n";
}

void ASTBuilder::printRPN(ASTNode *n, std::ostream &out, StrMode mode)
{
    if (!n)
        return;
    if (n->kind == "Bin")
    {
        ASTNode *L = n->leftChild, *R = L ? L->rightSibling : nullptr;
        printRPN(L, out, mode);
        out << "   ";
        printRPN(R, out, mode);
        out << "   " << n->text;
        return;
    }
    if (n->kind == "Un")
    {
        printRPN(n->leftChild, out, mode);
        out << "   " << n->text;
        return;
    }
    if (n->kind == "Id" || n->kind == "Int" || n->kind == "Bool")
    {
        out << n->text;
        return;
    }
    if (n->kind == "ArrAt")
    {
        out << n->text << "   [   ";
        printRPN(n->leftChild, out, mode);
        out << "   ]";
        return;
    }
    if (n->kind == "Str")
    {
        if (mode == StrMode::Bare) {
            std::string text = n->text;
            //remove trailin spacee
            while (!text.empty() && text.back() == ' ') {
                text.pop_back();
            }
            out << text;
        }
        else {
            out << "\"   " << n->text << "   \"";
        }
        return;
    }
    if (n->kind == "Char")
    {
        out << "'   " << n->text << "   '";
        return;
    }
    if (n->kind == "Call")
    {
        out << n->text << "   (   ";
        bool first = true;
        for (ASTNode *a = n->leftChild; a; a = a->rightSibling)
        {
            if (!first)
                out << "   ,   ";
            printRPN(a, out, mode);
            first = false;
        }
        out << "   )";
        return;
    }
    out << n->text;
}
