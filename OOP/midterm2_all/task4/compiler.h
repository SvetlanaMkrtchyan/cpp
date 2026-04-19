#pragma once
#include "ast.h"
#include "instructions.h"
#include <map>

struct BreakPatch {
    int pos;
};

int compile(
    const ExprNode*      node,
    vector<Instruction>& prog,
    vector<double>&      constPool,
    int&                 nextReg,
    map<string, int>&    varMap,
    vector<BreakPatch>*  breakPatches = nullptr
);
