#pragma once
#include <string>
#include <vector>
using namespace std;

enum TokenType {
    NUMBER, VARIABLE, OPERATOR,
    LPAREN, RPAREN, LBRACE, RBRACE,
    COMMA, SEMICOLON,
    KW_IF, KW_ELSE, KW_RETURN, KW_INT, KW_DOUBLE, KW_WHILE
};

struct Token {
    TokenType type;
    string    value;
};

extern vector<Token> tokenList;
extern int           curIdx;

Token& cur();
bool   atEnd();
Token  consume();
bool   peek(TokenType t);
bool   peekVal(const string& v);

void tokenize(const string& input);
