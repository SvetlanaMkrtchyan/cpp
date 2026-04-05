#include "executor.h"
#include <iostream>
#include <iomanip>
using namespace std;

double execute(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    vector<double>&            varValues,
    double*                    regs,
    int                        startPC
) {
    int pc = startPC;
    while (pc < (int)prog.size()) {
        const auto& inst = prog[pc++];
        switch ((OpCode)inst.op) {
        case OpCode::MOV_CONST:    regs[inst.resIdx] = constPool[inst.leftIdx]; break;
        case OpCode::LOAD_VAR:     regs[inst.resIdx] = varValues[inst.leftIdx]; break;
        case OpCode::STORE_VAR:    varValues[inst.leftIdx] = regs[inst.resIdx]; break;
        case OpCode::ADD:          regs[inst.resIdx] = regs[inst.leftIdx] + regs[inst.rightIdx]; break;
        case OpCode::SUB:          regs[inst.resIdx] = regs[inst.leftIdx] - regs[inst.rightIdx]; break;
        case OpCode::MUL:          regs[inst.resIdx] = regs[inst.leftIdx] * regs[inst.rightIdx]; break;
        case OpCode::DIV:          regs[inst.resIdx] = regs[inst.rightIdx] != 0
                                       ? regs[inst.leftIdx] / regs[inst.rightIdx] : 0; break;
        case OpCode::CMP_EQ:       regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ:      regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:       regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:       regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LTE:      regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GTE:      regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::JMP:          pc = inst.resIdx; break;
        case OpCode::JMP_IF_FALSE: if (regs[inst.resIdx] == 0.0) pc = inst.leftIdx; break;
        case OpCode::CALL: {
            auto& fe = funcTable[inst.leftIdx];
            vector<double> args;
            for (int i = 0; i < (int)fe.params.size(); i++)
                args.push_back(regs[inst.rightIdx + i]);
            regs[inst.resIdx] = callFunction(inst.leftIdx, args);
            break;
        }
        case OpCode::RET:  return regs[inst.resIdx];
        case OpCode::HALT: return regs[inst.resIdx];
        }
    }
    return 0;
}

double callFunction(int funcIdx, vector<double> args) {
    auto& fe = funcTable[funcIdx];
    double regs[1024] = {};
    vector<double> varVals(max((int)fe.constPool.size() + 10, 64), 0.0);
    for (int i = 0; i < (int)args.size() && i < (int)fe.params.size(); i++)
        varVals[i] = args[i];
    return execute(fe.code, fe.constPool, varVals, regs, 0);
}

void dumpProgram(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    const map<string, int>&    varMap,
    const string&              title
) {
    map<int, string> revVar;
    for (auto& [name, id] : varMap) revVar[id] = name;

    cout << "\n=== " << title << " ===\n";
    for (int i = 0; i < (int)prog.size(); i++) {
        const auto& inst = prog[i];
        OpCode op = (OpCode)inst.op;
        cout << "[" << setw(2) << i << "] " << left << setw(14) << opNames[inst.op];

        switch (op) {
        case OpCode::MOV_CONST:
            cout << "r" << inst.resIdx << ", " << constPool[inst.leftIdx];
            break;
        case OpCode::LOAD_VAR:
            cout << "r" << inst.resIdx << ", "
                 << (revVar.count(inst.leftIdx) ? revVar[inst.leftIdx] : "var" + to_string(inst.leftIdx));
            break;
        case OpCode::STORE_VAR:
            cout << (revVar.count(inst.leftIdx) ? revVar[inst.leftIdx] : "var" + to_string(inst.leftIdx))
                 << ", r" << inst.resIdx;
            break;
        case OpCode::ADD: case OpCode::SUB: case OpCode::MUL: case OpCode::DIV:
        case OpCode::CMP_EQ: case OpCode::CMP_NEQ:
        case OpCode::CMP_LT: case OpCode::CMP_GT:
        case OpCode::CMP_LTE: case OpCode::CMP_GTE:
            cout << "r" << inst.resIdx << ", r" << inst.leftIdx << ", r" << inst.rightIdx;
            break;
        case OpCode::JMP:
            cout << "-> pc=" << inst.resIdx;
            break;
        case OpCode::JMP_IF_FALSE:
            cout << "r" << inst.resIdx << ", -> pc=" << inst.leftIdx;
            break;
        case OpCode::CALL:
            cout << "r" << inst.resIdx << ", "
                 << (inst.leftIdx < funcTable.size() ? funcTable[inst.leftIdx].name : "?")
                 << "(r" << inst.rightIdx << ")";
            break;
        case OpCode::RET:
        case OpCode::HALT:
            cout << "r" << inst.resIdx << ", exit";
            break;
        }
        cout << "\n";
    }
}
