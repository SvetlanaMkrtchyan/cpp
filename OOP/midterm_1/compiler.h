#pragma once
#include "ast.h"
#include "instructions.h"
#include <map>

int compile(
    const ExprNode*      node,
    vector<Instruction>& prog,
    vector<double>&      constPool,
    int&                 nextReg,
    map<string, int>&    varMap
);
