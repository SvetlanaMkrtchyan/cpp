#pragma once
#include <string>
#include <vector>

using namespace std;

enum TokenType {
    NUMBER, VARIABLE, OPERATOR,
    LPAREN, RPAREN, LBRACE, RBRACE,
    COMMA, SEMICOLON, COLON,
    KW_IF, KW_ELSE, KW_RETURN,
    KW_INT, KW_DOUBLE, KW_VOID,
    KW_WHILE, KW_DO,
    KW_FOR,
    KW_SWITCH, KW_CASE, KW_DEFAULT, KW_BREAK,
    KW_INPUT,
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
