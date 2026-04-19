#pragma once
#include "instructions.h"
#include "memory.h"
#include <vector>
#include <map>
#include <set>
#include <string>

struct VMState {
    double   regs[NUM_REGS] = {};
    int      pc             = 0;
    bool     halted         = false;
    Memory*  mem            = nullptr;
};

double execute(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    vector<double>&            varValues,
    double*                    regs,
    int                        startPC = 0
);

double executeWithMemory(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    Memory&                    memory,
    int                        startPC = 0
);

double callFunction(int funcIdx, vector<double> args);

void dumpProgram(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    const map<string, int>&    varMap,
    const string&              title = "Main"
);

void runDebugger(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    vector<double>&            varValues,
    const map<string, int>&    varMap
);
