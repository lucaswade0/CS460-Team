# CS460 Compiler Project

## Project Structure

```
Assignment4/
├── main.cpp                          # Main entry point - orchestrates all phases
├── CommentRemover.h/cpp              # Assignment 1: Comment removal
├── Tokenizer.h/cpp                   # Assignment 2: Lexical analysis
├── CSTParser.h/cpp                   # Assignment 3: CST creation
├── SymbolTableBuilder.h/cpp          # Assignment 4: Symbol table
├── Makefile                          # Build system (use 'make')
└── file1.txt                         # Test input file

```


## Assignment 5: AST (Abstract Syntax Tree) - TO DO

**Objective**: Transform the CST into an Abstract Syntax Tree

### What Needs to Be Implemented:

1. **AST Conversion Module**
   - Create `ASTBuilder.h/cpp` 
   - Transform CST nodes into simplified AST nodes
   - Remove syntactic elements (punctuation, wrapper nodes)
   - Flatten expression trees into linear sequences

2. **Key Transformations Needed**:
   - `GlobalDecl`/`Declaration` → `DECLARATION`
   - `Block` → `BEGIN BLOCK` ... `END BLOCK`
   - `Assignment` → `ASSIGNMENT` with flattened expression
   - `IfStmt` → `IF` with condition and body
   - `FunctionCall` → Function name + arguments (no parentheses)
   - Remove: `;`, `(`, `)`, `{`, `}`, `[`, `]`, `,` nodes
   - Remove: `Parameters`, `Parameter` wrapper nodes
   - Flatten: All expression trees (`BinaryOp`, `UnaryOp`)

   

### Files to Create:
- `ASTBuilder.h` - AST builder interface
- `ASTBuilder.cpp` - AST conversion implementation
- Update `main.cpp` - Add AST building and printing after CST


## Building and Running

### Using Make:
```
make clean    # Clean build artifacts
make          # Compile project
./main file1.txt   # Run with test file
```




1. **Clone the repository**
   ```
   git clone <repository-url>
   cd Assignment4
   ```





