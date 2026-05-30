#pragma once
#include "ast.h"
#include "instructions.h"
#include <map>

struct BreakPatch {
    int pos;
};

struct ContinuePatch {
    int pos;
};

struct CompileInfo {
    int finalReg   = -1;
    int lastVarId  = -1;
    int instrCount = 0;
};

int compile(
    const ExprNode*      node,
    vector<Instruction>& prog,
    vector<double>&      constPool,
    int&                 nextReg,
    map<string, int>&    varMap,
    vector<BreakPatch>*  breakPatches = nullptr,
    vector<ContinuePatch>* continuePatches = nullptr,
    CompileInfo*         info = nullptr
);
