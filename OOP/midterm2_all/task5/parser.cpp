#include "parser.h"
#include "lexer.h"

static unique_ptr<ExprNode> parsePrimary() {
    if (atEnd()) return nullptr;

    if (peek(KW_INPUT)) {
        consume();
        string typeName = "int";
        if (peek(LPAREN)) {
            consume();
            if (peek(VARIABLE) || peek(KW_INT) || peek(KW_DOUBLE)) typeName = consume().value;
            if (peek(RPAREN)) consume();
        }
        return make_unique<ExprNode>(NodeKind::INPUT_NODE, typeName);
    }

    if (peek(VARIABLE) && curIdx+1 < (int)tokenList.size() && tokenList[curIdx+1].type == LPAREN) {
        string name = consume().value;
        consume();
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

    if (peekVal("-")) {
        consume();
        auto zero = make_unique<ExprNode>(0.0);
        auto right = parsePrimary();
        auto node = make_unique<ExprNode>(NodeKind::OP_NODE, "-");
        node->left  = move(zero);
        node->right = move(right);
        return node;
    }

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
    while (!atEnd() && (peekVal("*") || peekVal("/") || peekVal("%"))) {
        string op = consume().value;
        auto p = make_unique<ExprNode>(NodeKind::OP_NODE, op);
        p->left  = move(l);
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
        p->left  = move(l);
        p->right = parseTerm();
        l = move(p);
    }
    return l;
}

static unique_ptr<ExprNode> parseShift() {
    auto l = parseAddSub();
    while (!atEnd() && (peekVal("<<") || peekVal(">>"))) {
        string op = consume().value;
        auto p = make_unique<ExprNode>(NodeKind::OP_NODE, op);
        p->left  = move(l);
        p->right = parseAddSub();
        l = move(p);
    }
    return l;
}

static unique_ptr<ExprNode> parseComparison() {
    auto l = parseShift();
    static vector<string> ops = {"==", "!=", "<", ">", "<=", ">="};
    while (!atEnd()) {
        bool found = false;
        for (auto& op : ops) if (peekVal(op)) { found = true; break; }
        if (!found) break;
        string op = consume().value;
        auto p = make_unique<ExprNode>(NodeKind::CMP_NODE, op);
        p->left  = move(l);
        p->right = parseShift();
        l = move(p);
    }
    return l;
}

static unique_ptr<ExprNode> parseBitAnd() {
    auto l = parseComparison();
    while (!atEnd() && peekVal("&")) {
        consume();
        auto p = make_unique<ExprNode>(NodeKind::OP_NODE, "&");
        p->left  = move(l);
        p->right = parseComparison();
        l = move(p);
    }
    return l;
}

static unique_ptr<ExprNode> parseBitOr() {
    auto l = parseBitAnd();
    while (!atEnd() && peekVal("|")) {
        consume();
        auto p = make_unique<ExprNode>(NodeKind::OP_NODE, "|");
        p->left  = move(l);
        p->right = parseBitAnd();
        l = move(p);
    }
    return l;
}

unique_ptr<ExprNode> parseExpr() { return parseBitOr(); }

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

unique_ptr<ExprNode> parseStatement() {
    if (atEnd()) return nullptr;

    if (peek(KW_BREAK)) {
        consume();
        if (peek(SEMICOLON)) consume();
        return make_unique<ExprNode>(NodeKind::BREAK_NODE, "break");
    }

    if (peek(KW_WHILE)) {
        consume();
        if (peek(LPAREN)) consume();
        auto cond = parseExpr();
        if (peek(RPAREN)) consume();
        auto body = parseBlock();
        auto node = make_unique<ExprNode>(NodeKind::WHILE_NODE, "while");
        node->cond = move(cond);
        node->left = move(body);
        return node;
    }

    if (peek(KW_DO)) {
        consume();
        auto body = parseBlock();
        if (peek(KW_WHILE)) consume();
        if (peek(LPAREN)) consume();
        auto cond = parseExpr();
        if (peek(RPAREN)) consume();
        if (peek(SEMICOLON)) consume();
        auto node = make_unique<ExprNode>(NodeKind::DO_WHILE_NODE, "do");
        node->cond = move(cond);
        node->left = move(body);
        return node;
    }

    if (peek(KW_FOR)) {
        consume();
        if (peek(LPAREN)) consume();
        auto init = parseAssignOrExpr();
        if (peek(SEMICOLON)) consume();
        auto cond = parseExpr();
        if (peek(SEMICOLON)) consume();
        auto upd = parseAssignOrExpr();
        if (peek(RPAREN)) consume();
        auto body = parseBlock();
        auto node = make_unique<ExprNode>(NodeKind::FOR_NODE, "for");
        node->cond   = move(cond);
        node->left   = move(body);
        node->update = move(upd);
        node->children.push_back(move(init));
        return node;
    }

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

    if (peek(KW_SWITCH)) {
        consume();
        if (peek(LPAREN)) consume();
        auto expr = parseExpr();
        if (peek(RPAREN)) consume();
        if (peek(LBRACE)) consume();
        auto node = make_unique<ExprNode>(NodeKind::SWITCH_NODE, "switch");
        node->left = move(expr);
        while (!atEnd() && !peek(RBRACE)) {
            if (peek(KW_CASE)) {
                consume();
                auto val = parseExpr();
                if (peek(COLON)) consume();
                auto caseNode = make_unique<ExprNode>(NodeKind::CASE_NODE, "case");
                caseNode->cond = move(val);
                auto body = make_unique<ExprNode>(NodeKind::SEQUENCE_NODE, "seq");
                while (!atEnd() && !peek(KW_CASE) && !peek(KW_DEFAULT) && !peek(RBRACE)) {
                    auto s = parseStatement();
                    if (s) body->children.push_back(move(s));
                }
                caseNode->left = move(body);
                node->children.push_back(move(caseNode));
            } else if (peek(KW_DEFAULT)) {
                consume();
                if (peek(COLON)) consume();
                auto defNode = make_unique<ExprNode>(NodeKind::DEFAULT_NODE, "default");
                auto body = make_unique<ExprNode>(NodeKind::SEQUENCE_NODE, "seq");
                while (!atEnd() && !peek(KW_CASE) && !peek(RBRACE)) {
                    auto s = parseStatement();
                    if (s) body->children.push_back(move(s));
                }
                defNode->left = move(body);
                node->children.push_back(move(defNode));
            } else {
                consume();
            }
        }
        if (peek(RBRACE)) consume();
        return node;
    }

    if (peek(KW_RETURN)) {
        consume();
        auto node = make_unique<ExprNode>(NodeKind::RETURN_NODE, "return");
        if (!peek(SEMICOLON) && !peek(RBRACE)) node->left = parseExpr();
        if (peek(SEMICOLON)) consume();
        return node;
    }

    if (peek(KW_INT) || peek(KW_DOUBLE) || peek(KW_VOID)) {
        string typeName = consume().value;
        if (!peek(VARIABLE)) return nullptr;
        string name = consume().value;

        if (peek(LPAREN)) {
            consume();
            auto node = make_unique<ExprNode>(NodeKind::FUNC_DEF_NODE, name);
            node->text = name;
            while (!atEnd() && !peek(RPAREN)) {
                if (peek(KW_INT) || peek(KW_DOUBLE) || peek(KW_VOID)) consume();
                if (peek(VARIABLE)) node->params.push_back(consume().value);
                if (peek(COMMA)) consume();
            }
            if (peek(RPAREN)) consume();
            node->left = parseBlock();
            return node;
        }

        auto node = make_unique<ExprNode>(NodeKind::VAR_DECL_NODE, name);
        node->text = name;
        if (peekVal("=")) {
            consume();
            node->left = parseExpr();
        }
        if (peek(SEMICOLON)) consume();
        return node;
    }

    auto s = parseAssignOrExpr();
    if (peek(SEMICOLON)) consume();
    return s;
}
