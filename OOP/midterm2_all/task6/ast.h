#pragma once
#include <string>
#include <vector>
#include <memory>

using namespace std;

enum class NodeKind {
    NUM_NODE,
    VAR_NODE,
    OP_NODE,
    CMP_NODE,
    IF_NODE,
    WHILE_NODE,
    DO_WHILE_NODE,
    FOR_NODE,
    SWITCH_NODE,
    CASE_NODE,
    DEFAULT_NODE,
    SEQUENCE_NODE,
    RETURN_NODE,
    ASSIGN_NODE,
    VAR_DECL_NODE,
    FUNC_DEF_NODE,
    FUNC_CALL_NODE,
    BR_NODE,
    INPUT_NODE,
    BREAK_NODE,
};

struct ExprNode {
    NodeKind kind;
    string   text;
    double   val;

    vector<unique_ptr<ExprNode>> children;
    unique_ptr<ExprNode> left;
    unique_ptr<ExprNode> right;
    unique_ptr<ExprNode> cond;
    unique_ptr<ExprNode> update;
    vector<string> params;

    ExprNode(NodeKind k, string t) : kind(k), text(t), val(0) {}
    ExprNode(double v)             : kind(NodeKind::NUM_NODE), text(""), val(v) {}
};
