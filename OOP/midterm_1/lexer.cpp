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
        {"int", KW_INT}, {"double", KW_DOUBLE}
    };
    static map<char, TokenType> singles = {
        {'(', LPAREN}, {')', RPAREN}, {'{', LBRACE}, {'}', RBRACE},
        {',', COMMA},  {';', SEMICOLON}
    };
    static string cmpChars = "<>=!";

    for (int i = 0; i < (int)input.size(); ) {
        if (isspace(input[i])) { 
            i++; 
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

        if (i+1 < (int)input.size() && cmpChars.find(input[i]) != string::npos && input[i+1] == '=') {
            tokenList.push_back({OPERATOR, string(1, input[i]) + "="});
            i += 2; continue;
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
