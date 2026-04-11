#pragma once
#include "instructions.h"
#include <vector>
#include <map>
#include <string>

double execute(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    vector<double>&            varValues,
    double*                    regs,
    int                        startPC = 0
);

double callFunction(int funcIdx, vector<double> args);

void dumpProgram(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    const map<string, int>&    varMap,
    const string&              title = "Main"
);