#include "lexer.h"
#include <map>

vector<Token> tokenList;
int           curIdx = 0;

Token& cur()                    { return tokenList[curIdx]; }
bool   atEnd()                  { return curIdx >= (int)tokenList.size(); }
Token  consume()                { return tokenList[curIdx++]; }
bool   peek(TokenType t)        { return !atEnd() && cur().type == t; }
bool   peekVal(const string& v) { return !atEnd() && cur().value == v; }

void tokenize(const string& input) {
    tokenList.clear();
    curIdx = 0;

    static map<string, TokenType> keywords = {
        {"if", KW_IF}, {"else", KW_ELSE}, {"return", KW_RETURN},
        {"int", KW_INT}, {"double", KW_DOUBLE}, {"void", KW_VOID},
        {"while", KW_WHILE}, {"do", KW_DO},
        {"for", KW_FOR},
        {"switch", KW_SWITCH}, {"case", KW_CASE},
        {"default", KW_DEFAULT}, {"break", KW_BREAK},
        {"input", KW_INPUT},
    };

    static map<char, TokenType> singles = {
        {'(', LPAREN}, {')', RPAREN}, {'{', LBRACE}, {'}', RBRACE},
        {',', COMMA},  {';', SEMICOLON}, {':', COLON},
    };

    static string cmpChars = "<>=!";

    for (int i = 0; i < (int)input.size(); ) {
        if (isspace(input[i])) { i++; continue; }

        if (input[i] == '/' && i+1 < (int)input.size() && input[i+1] == '/') {
            while (i < (int)input.size() && input[i] != '\n') i++;
            continue;
        }

        if (isdigit(input[i]) || (input[i] == '.' && i+1 < (int)input.size() && isdigit(input[i+1]))) {
            string t;
            while (i < (int)input.size() && (isdigit(input[i]) || input[i] == '.')) t += input[i++];
            tokenList.push_back({NUMBER, t});
            continue;
        }

        if (isalpha(input[i]) || input[i] == '_') {
            string t;
            while (i < (int)input.size() && (isalnum(input[i]) || input[i] == '_')) t += input[i++];
            auto it = keywords.find(t);
            tokenList.push_back({it != keywords.end() ? it->second : VARIABLE, t});
            continue;
        }

        if (i+1 < (int)input.size()) {
            string two = string(1, input[i]) + input[i+1];
            if (two == "==" || two == "!=" || two == "<=" || two == ">=" ||
                two == "<<" || two == ">>" || two == "&&" || two == "||") {
                tokenList.push_back({OPERATOR, two});
                i += 2;
                continue;
            }
        }

        auto it = singles.find(input[i]);
        if (it != singles.end()) {
            tokenList.push_back({it->second, string(1, input[i])});
        } else {
            tokenList.push_back({OPERATOR, string(1, input[i])});
        }
        i++;
    }
}
