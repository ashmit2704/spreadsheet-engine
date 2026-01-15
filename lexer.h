#pragma once
#include <string>

using namespace std;

enum class TokenType {
    NUMBER, 
    CELL,
    PLUS, 
    MINUS, 
    MUL, 
    DIV,
    LPAREN, 
    RPAREN, 
    END
};

struct Token {
    TokenType type;
    string text;
};

class Lexer {
    string input;
    int pos = 0;
public:
    Lexer(const string& s);
    Token nextToken();
};
