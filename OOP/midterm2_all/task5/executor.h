#pragma once
#include "instructions.h"
#include "memory.h"
#include <vector>
#include <map>
#include <set>
#include <string>

struct VMState {
    double   regs[NUM_REGS] = {};
    uint32_t IR             = 0;
    int      pc             = 0;
    bool     halted         = false;
    Memory*  mem            = nullptr;
};

struct CpuState {
    uint64_t ip = 0;
    uint64_t sp = 0;
    uint64_t bp = 0;
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

double executeWithStack(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    Memory&                    memory,
    CpuState&                  cpu
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

double executeRV32I(
    const vector<uint32_t>& rawProgram,
    Memory&                 memory,
    uint32_t                startPC = 0
);
