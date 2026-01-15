#pragma once
#include "lexer.h"
#include "ast.h"

using namespace std;

class Parser {
    Lexer lexer;
    Token current;

    unique_ptr<ASTNode> parseExpression();
    unique_ptr<ASTNode> parseTerm();
    unique_ptr<ASTNode> parseFactor();

public:
    Parser(const string& input);
    unique_ptr<ASTNode> parse();
};
