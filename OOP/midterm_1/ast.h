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
    SEQUENCE_NODE,  
    RETURN_NODE,    
    ASSIGN_NODE,  
    FUNC_DEF_NODE,  
    FUNC_CALL_NODE 
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