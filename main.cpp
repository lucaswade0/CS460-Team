#include "CommentRemover.h"
#include "Tokenizer.h"
#include "CSTParser.h"
#include "SymbolTableBuilder.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char* argv[]) {
    string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        filename = "file1.txt";
    }

    // Read input file
    ifstream inFile(filename);
    if (!inFile) {
        cerr << "Can't open file" << endl;
        return 1;
    }

    stringstream buffer;
    buffer << inFile.rdbuf();
    string inputContent = buffer.str();
    inFile.close();

    // Assignment 1: Remove comments
    string cleanedContent = CommentRemover::removeComments(inputContent);

    // Assignment 2: Tokenize
    vector<Token> tokens = Tokenizer::tokenize(cleanedContent);

    // Assignment 3: Build CST
    CSTParser parser(tokens);
    TreeNode* cst = parser.parse();

    // Assignment 4: Build Symbol Table
    SymbolTable table;
    vector<ParameterList> parameterLists;
    int scope = 0;
    SymbolTableBuilder::buildSymbolTable(cst, table, scope, parameterLists);

    // Print symbol table
    table.print();

    // Print parameter lists
    SymbolTableBuilder::printParameterLists(parameterLists);

    // Write CST output
    std::ofstream outFile("output.txt");
    CSTParser::printCST(tokens, outFile);
    outFile.close();

    return 0;
}
