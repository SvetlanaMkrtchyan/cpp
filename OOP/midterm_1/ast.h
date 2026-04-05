#pragma once
#include <string>
#include <vector>
#include <memory>
using namespace std;

enum class NodeKind {
    NUM_NODE,       // num
    VAR_NODE,       // var
    OP_NODE,        // (+,-,*,/)
    CMP_NODE,       // (==,!=,<,>,<=,>=)
    IF_NODE,        // if / else if / else
    SEQUENCE_NODE,  // { stmt; stmt; ... }
    RETURN_NODE,    // return expr
    ASSIGN_NODE,    // var = expr
    FUNC_DEF_NODE,  // int name(params) { body }
    FUNC_CALL_NODE  // name(args)
};

struct ExprNode {
    NodeKind kind;
    string   text;      
    double   val;     

    vector<unique_ptr<ExprNode>> children; 
    unique_ptr<ExprNode> left;
    unique_ptr<ExprNode> right;
    unique_ptr<ExprNode> cond;             
    vector<string> params;               

    ExprNode(NodeKind k, string t) : kind(k), text(t), val(0) {}
    ExprNode(double v)             : kind(NodeKind::NUM_NODE), text(""), val(v) {}
};