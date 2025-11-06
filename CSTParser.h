#ifndef CSTPARSER_H
#define CSTPARSER_H

#include "Tokenizer.h"
#include <vector>
#include <string>
#include <fstream>

using namespace std;

/**
 * Description: COntructor for TreeNode structure. Creates a node in the CST with
 *              the given value and line number
 * Params:      Val: The string value stored in node(token val or node type)
 *              ln: The line number where this element appears in source code
 * Pre:         Val: can be any string (token value, keyword, or node type name)
 *              ln: should be >= 0
 * Post:        Creates a new TreeNode with the specified value and line number
 *              leftChild is initialized to nullptr
 *              rightSibling uis initialized to nullptr
 * Returns:     N/A (COnstuvtor)
 *
 */
struct TreeNode {
    string value;
    int line;
    TreeNode* leftChild;
    TreeNode* rightSibling;

    TreeNode(string val, int ln = 0);
};
/*
 * DEFINITION:  CSTParser::CSTParser(vector<Token> toks)
 *
 * DESCRIPTION: Contructor for CSTParser class. Initializes the parser with a vector of tokes to parse.
 *
 * PARAMS:      toks: A vector of Token objects representing the tokenized input
 *
 * Pre:         toks: contains valid Token objects from the Tokenizer
 *              toks: should not be empty
 *
 * Post:        Parser is initialized with the token stream
 *              Current index is set to 0
 *
 * Returns: N/A (Contructor)
 *
 */
class CSTParser {
private:
    vector<Token> tokens;
    size_t current;

    /**
     * @Description: Returns the current token without consuming it.
     *               Automatically skips over whitespace and newline
     *               tokens to get the next significant token.
     * @Params:      NONE
     * @Pre:         Parser has been initialized with a token stream
     *               token vector is valid
     * @return       TOken: The current token (after skipping whitespace/newlines
     *               Returns TOken{END_OF_FILE, "", 0, 0} if at end of stream
     */
    Token peek();

    /**
     * @Description Returns the current token and moves to the next token in the stream.
     *              Calls peek() to skip whitespace, then increments the current position.
     * @Params      NONE
     * @Pre         -Parser has been initialized with a token stream
     *              -Token vector is valid
     * @Post        -Current index is incremented by 1
     *              -The current token is consumed
     * @returns     -Token: The token that was current before advancing
     */
    Token advance();

    /**
     * @Description -Checks if the current token matches the specified type without consuming it.
     *
     * @param type  The TokenType to check against
     *
     * @Pre         -Parser has been initialized with a token stream
     *              -type is a valid TokenType enum value
     *
     * @Post        -No state changes
     *              -No tokens are consumed
     *
     * @returns     -bool: true if current token matches type, false otherwise
     */
    bool check(TokenType type);

    /**
     * @Description     -Checks if the current token's value matches the specified string
     *                   without consuming the token.
     *
     * @param value     -The string value to match against
     *
     * @Pre             -Parser has been initialized with a token stream
     *                  -Value is a valid string
     *
     * @Post            -No state changes
     *                  -No tokens are consumed
     *
     * @returns         -bool: true if current token value matches the input string,
     *                  false otherwise
     */
    bool match(const string& value);

    /**
     * @Description     Expects the current token to be of the specified type. If it matches,
     *                  advances and returns the token. If not, prints an error message and exits
     *
     *
     * @param type      The expected TokenType
     * @param errorMsg  Error message to display if expectation fails
     *
     * @Pre             Parser has been initialized w/ token stream
     * @Post            If token matches: current index advances, token is returned
     *                  If doesn't match: error message
     *
     * @returns         Token: The expected token (only if it matches)
     *                  Exits if doesnt match
     */
    Token expect(TokenType type, const string& errorMsg);

    /**
     * @Description     Expects the current token to have the specified value. If it matches,
     *                  advances and returns the token. If not, prints error
     *
     * @param value     The expected token value
     * @param errorMsg  Error message to display
     *
     * @Pre             Parser has token stream
     *                  Value is a valid string
     *
     * @Post            -If token value matches: current index advances, token is returned
     *                  -If token value doesnt match: Error
     *
     * @returns         Token: The expected token if value matches
     *                  Exits if !match
     */
    Token expectValue(const string& value, const string& errorMsg);

    /**
     * @Description     Adds a child to a parent node in the CST. Uses left-child, right-sibling
     *                  representation. The child becomes either the first child or the last
     *                  sibling of existing children.
     *
     * @param parent    Pointer to the parent TreeNode
     * @param child     Pointer to the child TreeNode to add
     *
     * @Pre             Parent and child can be nullptr (function handles this case)
     *                  If not nullptr, parent and child must point to valid TreeNode objects
     *
     * @Post            if either parent or child is nullptr: no changes made
     *                  if parent has no children: child becomes leftCHild of parent
     *                  if parent has children: child is added as rightSibling of last child
     *                  Tree structure is modified to include the new child
     */
    void addChild(TreeNode* parent, TreeNode* child);

    /**
     * @Description     Entry point for parsing. Parses the entire program which consists
     *                  of global declarations and function/procedure definitions
     *
     * @Pre             Parser has valid token stream
     *                  Token stream represents a valid program structure
     *
     * @Post            Entire program is parsed into a CST
     *                  Current index points to END_OF_FILE token
     *                  EXITS: if syntax error is encountered
     *
     * @returns         TreeNode*: Pointer to root node labeled "Program" with all global
     *                             declarations and functions as children
     *                  Exits program on syntax error
     */
    TreeNode* parseProgram();

    /**
    * @Description     Parses a global variable declaration statement. Handles multiple variables
    *                  of the same type declared on one line (comma-separated).
    *
    * @Pre             Current token is a type keyword (int, char, bool, or void)
    *                  Valid variable declarations follow the type
    *
    * @Post            Global Declaration is parsed into a CST subtree
    *                  Current index advances past the semicolon
    *                  Exits if syntax error
    *
    * @returns         TreeNode* Pointer to node labeled "GlobalDecl" containing:
    *                            - Type token as first child
    *                            - Variable declarators and commas as subsequent children
    *                            - Semicolon as last child
    *                  Exits on syntax error
    */
    TreeNode* parseGlobalDeclaration();

    /**
     * @Description     Parses a function or procedure definition. Functions have a return type;
     *                  procedures do not. Validates that the function/procedure name is not a
     *                  reserved word.
     *
     * @Pre             Current token is "function" or "procedure"
     *                  Valid identifier, parameters, and block follow
     *
     * @Post            Function/procedure is parsed into a CST subtree
     *                  Current index advances past the closing brace of the block
     *                  Exits if syntax error or reserved word used as function name
     *
     * @returns         TreeNode*: Pointer to node labeled "function" or "procedure" containing:
     *                             - Keyword as first child
     *                             - Return type (functions only) as second child
     *                             - Function name as child
     *                             - Left parenthesis, Parameters node, Right parenthesis
     *                             - Block node
     *                  Exits on syntax error or reserved word violation
     */
    TreeNode* parseFunctionOrProcedure();

    /**
     * @Description     Parses the parameter list of a function or procedure. Handles void
     *                  parameters (no parameters) and comma-separated parameter lists.
     *
     * @Pre             Current token is either 'void' or a type keyword for parameters
     *                  Parameters are properly formatted
     *
     * @Post            Parameter list is parsed into a CST subtree
     *                  Current index advances past the last parameter
     *
     * @returns         TreeNode*: Pointer to node labeled "Parameters" containing:
     *                             - Single "void" child if no parameters
     *                             - OR parameter nodes and commas as children for parameter list
     */
    TreeNode* parseParameters();

    /**
     * @Description     Parses a single parameter declaration. Parameters can be scalar or array
     *                  types. Validates that parameter name is not a reserved word.
     *
     * @Pre             Current token is a type keyword (int, char, bool)
     *                  Valid identifier follows
     *                  Optional array brackets may follow
     *
     * @Post            Parameter is parsed into a CST subtree
     *                  Current index advances past the parameter
     *                  Exits if syntax error or reserved word used as parameter name
     *
     * @returns         TreeNode*: Pointer to node labeled "Parameter" containing:
     *                             - Type as first child
     *                             - Parameter name as second child
     *                             - Optional: '[', optional size, ']' for array parameters
     *                  Exits on syntax error or reserved word violation
     */
    TreeNode* parseParameter();

    /**
     * @Description     Parses a code block enclosed in braces. Blocks contain local variable
     *                  declarations followed by statements.
     *
     * @Pre             Current token is '{'
     *                  Block contains valid declarations and statements
     *
     * @Post            Block is parsed into a CST subtree
     *                  Current index advances past the closing '}'
     *                  Exits if syntax error (missing braces, invalid statements, etc.)
     *
     * @returns         TreeNode*: Pointer to node labeled "Block" containing:
     *                             - '{' as first child
     *                             - Declaration nodes (if any)
     *                             - Statement nodes
     *                             - '}' as last child
     *                  Exits on syntax error
     */
    TreeNode* parseBlock();

    /**
     * @Description     Parses a local variable declaration statement. Handles multiple variables
     *                  of the same type declared on one line (comma-separated).
     *
     * @Pre             Current token is a type keyword (int, char, or bool)
     *                  Valid variable declarators follow
     *
     * @Post            Declaration is parsed into a CST subtree
     *                  Current index advances past the semicolon
     *                  Exits if syntax error
     *
     * @returns         TreeNode*: Pointer to node labeled "Declaration" containing:
     *                             - Type as first child
     *                             - Variable declarators and commas as subsequent children
     *                             - Semicolon as last child
     *                  Exits on syntax error
     */
    TreeNode* parseDeclaration();

    /**
     * @Description     Parses a single variable declarator (variable name with optional array size).
     *                  Validates that variable name is not a reserved word and that array sizes
     *                  are positive integers.
     *
     * @Pre             Current token is an identifier
     *                  Optional array brackets with size may follow
     *
     * @Post            Variable declarator is parsed into a CST subtree
     *                  Current index advances past the declarator
     *                  Exits if:
     *                  - Variable name is a reserved word
     *                  - Array size is not a positive integer
     *                  - Array size is negative or zero
     *
     * @returns         TreeNode*: Pointer to node labeled "VarDecl" containing:
     *                             - Variable name as first child
     *                             - Optional: '[', array size, ']' for array variables
     *                  Exits on syntax error or validation failure
     */
    TreeNode* parseVariableDeclarator();

    /**
     * @Description     Dispatcher function that determines the type of statement and calls the
     *                  appropriate parsing function. Handles if, while, for, return, block,
     *                  and expression statements.
     *
     * @Pre             Current token begins a valid statement
     *
     * @Post            Statement is parsed into a CST subtree
     *                  Current index advances past the statement
     *                  Exits if token doesn't match any statement type
     *
     * @returns         TreeNode*: Pointer to appropriate statement node returned by:
     *                             parseIfStatement(), parseWhileStatement(), parseForStatement(),
     *                             parseReturnStatement(), parseBlock(), or parseExpressionStatement()
     *                  Exits if unexpected token encountered
     */
    TreeNode* parseStatement();

    /**
     * @Description     Parses an if statement with optional else clause. The condition must be
     *                  enclosed in parentheses.
     *
     * @Pre             Current token is 'if'
     *                  Valid condition expression and statements follow
     *
     * @Post            If statement is parsed into a CST subtree
     *                  Current index advances past the statement(s)
     *                  Exits if syntax error (missing parentheses, etc.)
     *
     * @returns         TreeNode*: Pointer to node labeled "IfStmt" containing:
     *                             - 'if' keyword
     *                             - '(', Condition expression, ')'
     *                             - Then statement
     *                             - Optional: 'else', else statement
     *                  Exits on syntax error
     */
    TreeNode* parseIfStatement();

    /**
     * @Description     Parses a while loop statement. The condition must be enclosed in parentheses.
     *
     * @Pre             Current token is 'while'
     *                  Valid condition expression and statement follow
     *
     * @Post            While statement is parsed into a CST subtree
     *                  Current index advances past the loop body
     *                  Exits if syntax error (missing parentheses, etc.)
     *
     * @returns         TreeNode*: Pointer to node labeled "WhileStmt" containing:
     *                             - 'while' keyword
     *                             - '(', Condition expression, ')'
     *                             - Loop body statement
     *                  Exits on syntax error
     */
    TreeNode* parseWhileStatement();

    /**
     * @Description     Parses a for loop statement with initialization, condition, and update
     *                  expressions.
     *
     * @Pre             Current token is 'for'
     *                  Valid initialization, condition, update, and body follow
     *
     * @Post            For statement is parsed into a CST subtree
     *                  Current index advances past the loop body
     *                  Exits if syntax error (missing parentheses, semicolons, etc.)
     *
     * @returns         TreeNode*: Pointer to node labeled "ForStmt" containing:
     *                             - 'for' keyword
     *                             - '(', Initialization assignment, ';'
     *                             - Condition expression, ';'
     *                             - Update assignment, ')'
     *                             - Loop body statement
     *                  Exits on syntax error
     */
    TreeNode* parseForStatement();

    /**
     * @Description     Parses a return statement with an expression.
     *
     * @Pre             Current token is 'return'
     *                  Valid expression and semicolon follow
     *
     * @Post            Return statement is parsed into a CST subtree
     *                  Current index advances past the semicolon
     *                  Exits if syntax error (missing semicolon, etc.)
     *
     * @returns         TreeNode*: Pointer to node labeled "ReturnStmt" containing:
     *                             - 'return' keyword
     *                             - Return value expression
     *                             - ';'
     *                  Exits on syntax error
     */
    TreeNode* parseReturnStatement();

    /**
     * @Description     Parses an expression statement, which can be either an assignment or a
     *                  function call. Uses lookahead to determine which type.
     *
     * @Pre             Current token is an identifier
     *                  Valid assignment or function call follows
     *
     * @Post            Expression statement is parsed into a CST subtree
     *                  Current index advances past the semicolon
     *                  Exits if syntax error or unexpected token
     *
     * @returns         TreeNode*: For assignment: Pointer to "Assignment" node with ';' appended
     *                             For function call: Pointer to "ExprStmt" wrapper containing
     *                             "FunctionCall" node and ';'
     *                  Exits on syntax error
     */
    TreeNode* parseExpressionStatement();

    /**
     * @Description     Parses an assignment statement. Supports both scalar and array element
     *                  assignments.
     *
     * @Pre             Current token is an identifier
     *                  Valid assignment operator and expression follow
     *
     * @Post            Assignment is parsed into a CST subtree (without semicolon)
     *                  Current index advances past the expression
     *                  Exits if syntax error
     *
     * @returns         TreeNode*: Pointer to node labeled "Assignment" containing:
     *                             - Variable name
     *                             - Optional: '[', index expression, ']' for array assignment
     *                             - '='
     *                             - Value expression
     *                  NOTE: Semicolon is NOT included in this node; caller consumes it
     *                  Exits on syntax error
     */
    TreeNode* parseAssignment();

    /**
     * @Description     Entry point for expression parsing. Delegates to parseLogicalOr() which
     *                  begins the precedence climbing for operator precedence.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Pointer to expression subtree (result from parseLogicalOr())
     */
    TreeNode* parseExpression();

    /**
     * @Description     Parses logical OR expressions (||). Left-associative binary operator with
     *                  lowest precedence.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Logical OR expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Single operand if no OR operator present
     *                             OR "BinaryOp" node with left operand, "||", and right operand
     */
    TreeNode* parseLogicalOr();

    /**
     * @Description     Parses logical AND expressions (&&). Left-associative binary operator with
     *                  precedence below OR, above equality.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Logical AND expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Single operand if no AND operator present
     *                             OR "BinaryOp" node with left operand, "&&", and right operand
     */
    TreeNode* parseLogicalAnd();

    /**
     * @Description     Parses equality expressions (==, !=). Left-associative binary operators
     *                  with precedence below AND, above relational.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Equality expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Single operand if no equality operator present
     *                             OR "BinaryOp" node with left operand, operator, and right operand
     */
    TreeNode* parseEquality();

    /**
     * @Description     Parses relational expressions (<, >, <=, >=). Left-associative binary
     *                  operators with precedence below equality, above additive.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Relational expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Single operand if no relational operator present
     *                             OR "BinaryOp" node with left operand, operator, and right operand
     */
    TreeNode* parseRelational();

    /**
     * @Description     Parses additive expressions (+, -). Left-associative binary operators
     *                  with precedence below relational, above multiplicative.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Additive expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Single operand if no additive operator present
     *                             OR "BinaryOp" node with left operand, operator, and right operand
     */
    TreeNode* parseAdditive();

    /**
     * @Description     Parses multiplicative expressions (*, /, %). Left-associative binary
     *                  operators with precedence below additive, above unary.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Multiplicative expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Single operand if no multiplicative operator present
     *                             OR "BinaryOp" node with left operand, operator, and right operand
     */
    TreeNode* parseMultiplicative();

    /**
     * @Description     Parses unary expressions (!, -). Right-associative unary operators with
     *                  precedence below multiplicative, above primary.
     *
     * @Pre             Current token begins a valid expression
     *
     * @Post            Unary expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *
     * @returns         TreeNode*: Single operand if no unary operator present
     *                             OR "UnaryOp" node with operator and operand as children
     */
    TreeNode* parseUnary();

    /**
     * @Description     Parses primary expressions (highest precedence). Includes literals,
     *                  identifiers, function calls, array access, and parenthesized expressions.
     *                  Uses lookahead to distinguish between identifiers, function calls, and
     *                  array access.
     *
     * @Pre             Current token begins a valid primary expression
     *
     * @Post            Primary expression is parsed into a CST subtree
     *                  Current index advances past the expression
     *                  Exits if unexpected token encountered
     *
     * @returns         TreeNode*: One of the following:
     *                             - Simple node with integer value (for INTEGER tokens)
     *                             - Simple node with identifier name (for variable references)
     *                             - "FunctionCall" node (for function calls)
     *                             - "ArrayAccess" node with name, '[', index, ']'
     *                             - "CharLiteral" node with "'", content, "'"
     *                             - "StringLiteral" node with "\"", content, "\""
     *                             - "ParenExpr" node with '(', expression, ')'
     *                  Exits on unexpected token
     */
    TreeNode* parsePrimary();

    /**
     * @Description     Parses a function call with optional comma-separated arguments.
     *
     * @Pre             Current token is an identifier (function name)
     *                  Valid parameter list enclosed in parentheses follows
     *
     * @Post            Function call is parsed into a CST subtree
     *                  Current index advances past the closing parenthesis
     *                  Exits if syntax error
     *
     * @returns         TreeNode*: Pointer to node labeled "FunctionCall" containing:
     *                             - Function name
     *                             - '('
     *                             - Argument expressions with commas (if any)
     *                             - ')'
     *                  Exits on syntax error
     */
    TreeNode* parseFunctionCall();

public:
    CSTParser(vector<Token> toks);

    /**
     * @Description     Public interface for parsing. Entry point that calls parseProgram() to
     *                  construct the complete CST.
     *
     * @Pre             Parser has been initialized with a valid token stream
     *
     * @Post            Entire program is parsed into a CST
     *                  Exits if any syntax errors are encountered
     *
     * @returns         TreeNode*: Pointer to the root of the CST (labeled "Program")
     *                  Exits program on syntax error
     */
    TreeNode* parse();

    /**
     * @Description     Recursively prints the CST in a tree format with indentation. Static
     *                  utility function that can be called without a parser instance.
     *
     * @param node      Pointer to the current node to print
     * @param depth     Current depth in the tree (for indentation, default 0)
     *
     * @Pre             node can be nullptr (handled gracefully)
     *                  depth should be >= 0
     *
     * @Post            Tree structure is printed to stdout
     *                  Each level is indented by 2 spaces
     *                  No modifications to the tree
     *
     * @returns         void (no return value)
     *                  Output is written to stdout
     */
    static void printTree(TreeNode* node, int depth = 0);

    /**
     * @Description     Prints tokens in a formatted layout that represents the CST structure.
     *                  Filters out whitespace and END_OF_FILE tokens, handles string literals
     *                  specially, and maintains proper line formatting.
     *
     * @param tokens    Vector of Token objects to print
     * @param out       Output file stream to write to
     *
     * @Pre             tokens is a valid vector of Token objects
     *                  out is an open, writable output stream
     *
     * @Post            Tokens are written to the output stream in formatted layout
     *                  Whitespace and END_OF_FILE tokens are filtered out
     *                  String literals are formatted with content separated from quotes
     *                  Each source line is preserved as a separate output line
     *
     * @returns         void (no return value)
     *                  Output is written to the provided ofstream
     */
    static void printCST(vector<Token>& tokens, ofstream& out);

    /**
     * @Description     Checks if a given word is a reserved keyword in the language. Static
     *                  utility function that can be called without a parser instance.
     *
     * @param word      The string to check
     *
     * @Pre             word is a valid string
     *
     * @Post            No state changes
     *                  Returns whether word is reserved
     *
     * @returns         bool: true if word is a reserved keyword, false otherwise
     *
     * RESERVED WORDS: int, char, void, bool, function, procedure, if, else,
     *                 while, for, return, printf, TRUE, FALSE
     */
    static bool isReservedWord(const string& word);
};

#endif
