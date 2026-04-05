#include "parser.h"
#include "lexer.h"

static unique_ptr<ExprNode> parsePrimary() {
    if (atEnd()) return nullptr;

    if (peek(VARIABLE) && curIdx+1 < (int)tokenList.size() && tokenList[curIdx+1].type == LPAREN) {
        string name = consume().value;
        consume();  // (
        auto node = make_unique<ExprNode>(NodeKind::FUNC_CALL_NODE, name);
        while (!atEnd() && !peekVal(")")) {
            node->children.push_back(parseExpr());
            if (peekVal(",")) consume();
        }
        if (!atEnd()) consume();  
        return node;
    }

    if (peek(NUMBER))   return make_unique<ExprNode>(stod(consume().value));
    if (peek(VARIABLE)) return make_unique<ExprNode>(NodeKind::VAR_NODE, consume().value);

    if (peek(LPAREN)) {
        consume();
        auto n = parseExpr();
        if (peek(RPAREN)) consume();
        return n;
    }
    return nullptr;
}

static unique_ptr<ExprNode> parseTerm() {
    auto l = parsePrimary();
    while (!atEnd() && (peekVal("*") || peekVal("/"))) {
        string op = consume().value;
        auto p = make_unique<ExprNode>(NodeKind::OP_NODE, op);
        p->left = move(l);
        p->right = parsePrimary();
        l = move(p);
    }
    return l;
}

static unique_ptr<ExprNode> parseAddSub() {
    auto l = parseTerm();
    while (!atEnd() && (peekVal("+") || peekVal("-"))) {
        string op = consume().value;
        auto p = make_unique<ExprNode>(NodeKind::OP_NODE, op);
        p->left = move(l);
        p->right = parseTerm();
        l = move(p);
    }
    return l;
}

static unique_ptr<ExprNode> parseComparison() {
    auto l = parseAddSub();
    static vector<string> ops = {"==", "!=", "<", ">", "<=", ">="};
    while (!atEnd()) {
        bool found = false;
        for (auto& op : ops) if (peekVal(op)) { found = true; break; }
        if (!found) break;
        string op = consume().value;
        auto p = make_unique<ExprNode>(NodeKind::CMP_NODE, op);
        p->left = move(l);
        p->right = parseAddSub();
        l = move(p);
    }
    return l;
}

unique_ptr<ExprNode> parseExpr() { return parseComparison(); }

static unique_ptr<ExprNode> parseAssignOrExpr() {
    if (peek(VARIABLE) && curIdx+1 < (int)tokenList.size() && tokenList[curIdx+1].value == "=") {
        string varName = consume().value;
        consume();  
        auto node = make_unique<ExprNode>(NodeKind::ASSIGN_NODE, varName);
        node->left = parseExpr();
        return node;
    }
    return parseExpr();
}

unique_ptr<ExprNode> parseStatement() {
    if (atEnd()) return nullptr;

    if (peek(KW_IF)) {
        consume();  
        if (peek(LPAREN)) consume();
        auto cond = parseExpr();
        if (peek(RPAREN)) consume();
        auto thenBody = parseBlock();
        auto node = make_unique<ExprNode>(NodeKind::IF_NODE, "if");
        node->cond  = move(cond);
        node->left  = move(thenBody);
        if (peek(KW_ELSE)) {
            consume();  
            node->right = peek(KW_IF) ? parseStatement() : parseBlock();
        }
        return node;
    }

    if (peek(KW_RETURN)) {
        consume();
        auto node = make_unique<ExprNode>(NodeKind::RETURN_NODE, "return");
        if (!peek(SEMICOLON) && !peek(RBRACE)) node->left = parseExpr();
        if (peek(SEMICOLON)) consume();
        return node;
    }

    if (peek(KW_INT) || peek(KW_DOUBLE)) {
        consume(); 
        if (!peek(VARIABLE)) return nullptr;
        string fname = consume().value;
        if (!peek(LPAREN)) return nullptr;
        consume();  
        auto node = make_unique<ExprNode>(NodeKind::FUNC_DEF_NODE, fname);
        while (!atEnd() && !peek(RPAREN)) {
            if (peek(KW_INT) || peek(KW_DOUBLE)) consume();  // param type
            if (peek(VARIABLE)) node->params.push_back(consume().value);
            if (peek(COMMA)) consume();
        }
        if (peek(RPAREN)) consume();
        node->left = parseBlock();
        return node;
    }

    auto s = parseAssignOrExpr();
    if (peek(SEMICOLON)) consume();
    return s;
}

unique_ptr<ExprNode> parseBlock() {
    if (peek(LBRACE)) {
        consume();  
        auto seq = make_unique<ExprNode>(NodeKind::SEQUENCE_NODE, "seq");
        while (!atEnd() && !peek(RBRACE)) {
            auto s = parseStatement();
            if (s) seq->children.push_back(move(s));
        }
        if (peek(RBRACE)) consume();
        return seq;
    }
    auto seq = make_unique<ExprNode>(NodeKind::SEQUENCE_NODE, "seq");
    auto s = parseStatement();
    if (s) seq->children.push_back(move(s));
    return seq;
}
