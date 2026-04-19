#include "executor.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <set>

using namespace std;

double execute(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    vector<double>&            varValues,
    double*                    regs,
    int                        startPC
) {
    uint32_t IR = 0;
    int pc = startPC;
    while (pc < (int)prog.size()) {

        IR = *reinterpret_cast<const uint32_t*>(&prog[pc]);
        const Instruction& inst = prog[pc++];
        switch ((OpCode)inst.op) {
        case OpCode::MOV_CONST:    regs[inst.resIdx] = constPool[inst.leftIdx]; break;
        case OpCode::LOAD_VAR:     regs[inst.resIdx] = varValues[inst.leftIdx]; break;
        case OpCode::STORE_VAR:
            while ((int)varValues.size() <= (int)inst.leftIdx) varValues.push_back(0.0);
            varValues[inst.leftIdx] = regs[inst.resIdx];
            break;
        case OpCode::ADD:      regs[inst.resIdx] = regs[inst.leftIdx] + regs[inst.rightIdx]; break;
        case OpCode::SUB:      regs[inst.resIdx] = regs[inst.leftIdx] - regs[inst.rightIdx]; break;
        case OpCode::MUL:      regs[inst.resIdx] = regs[inst.leftIdx] * regs[inst.rightIdx]; break;
        case OpCode::DIV:      regs[inst.resIdx] = regs[inst.rightIdx] != 0
                                   ? regs[inst.leftIdx] / regs[inst.rightIdx] : 0; break;
        case OpCode::MOD:      regs[inst.resIdx] = fmod(regs[inst.leftIdx], regs[inst.rightIdx]); break;
        case OpCode::BIT_AND:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] & (int64_t)regs[inst.rightIdx]; break;
        case OpCode::BIT_OR:   regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] | (int64_t)regs[inst.rightIdx]; break;
        case OpCode::BIT_XOR:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] ^ (int64_t)regs[inst.rightIdx]; break;
        case OpCode::SHL:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] << (int64_t)regs[inst.rightIdx]; break;
        case OpCode::SHR:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] >> (int64_t)regs[inst.rightIdx]; break;
        case OpCode::CMP_EQ:   regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ:  regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:   regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:   regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LTE:  regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GTE:  regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::JMP:      pc = inst.resIdx; break;
        case OpCode::JMP_REL:  pc += (int32_t)inst.resIdx; break;
        case OpCode::JMP_IF_FALSE: if (regs[inst.resIdx] == 0.0) pc = inst.leftIdx; break;
        case OpCode::JMP_IF_TRUE:  if (regs[inst.resIdx] != 0.0) pc += (int32_t)inst.leftIdx; break;
        case OpCode::BR_GT:    if (regs[inst.resIdx] >  regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT:    if (regs[inst.resIdx] <  regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_EQ:    if (regs[inst.resIdx] == regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE:   if (regs[inst.resIdx] >= regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE:   if (regs[inst.resIdx] <= regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GT_U:  if ((uint32_t)regs[inst.resIdx] >  (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT_U:  if ((uint32_t)regs[inst.resIdx] <  (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE_U: if ((uint32_t)regs[inst.resIdx] >= (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE_U: if ((uint32_t)regs[inst.resIdx] <= (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::READ_INT: {
            int v; cin >> v;
            regs[inst.resIdx] = v;
            break;
        }
        case OpCode::READ_DOUBLE: {
            double v; cin >> v;
            regs[inst.resIdx] = v;
            break;
        }
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
    double regs[NUM_REGS] = {};
    vector<double> varVals(max((int)fe.params.size() + 32, 64), 0.0);
    for (int i = 0; i < (int)args.size() && i < (int)fe.params.size(); i++)
        varVals[i] = args[i];
    return execute(fe.code, fe.constPool, varVals, regs, 0);
}

static string formatInst(int idx, const Instruction& inst, const vector<double>& constPool,
                          const map<int,string>& revVar) {
    ostringstream ss;
    OpCode op = (OpCode)inst.op;
    ss << "[" << setw(3) << idx << "]  " << left << setw(16) << opNames[inst.op];
    switch (op) {
    case OpCode::MOV_CONST:
        ss << "r" << inst.resIdx << ", " << constPool[inst.leftIdx]; break;
    case OpCode::LOAD_VAR:
        ss << "r" << inst.resIdx << ", "
           << (revVar.count(inst.leftIdx) ? revVar.at(inst.leftIdx) : "var"+to_string(inst.leftIdx)); break;
    case OpCode::STORE_VAR:
        ss << (revVar.count(inst.leftIdx) ? revVar.at(inst.leftIdx) : "var"+to_string(inst.leftIdx))
           << ", r" << inst.resIdx; break;
    case OpCode::ADD: case OpCode::SUB: case OpCode::MUL: case OpCode::DIV: case OpCode::MOD:
    case OpCode::BIT_AND: case OpCode::BIT_OR: case OpCode::BIT_XOR: case OpCode::SHL: case OpCode::SHR:
    case OpCode::CMP_EQ: case OpCode::CMP_NEQ: case OpCode::CMP_LT: case OpCode::CMP_GT:
    case OpCode::CMP_LTE: case OpCode::CMP_GTE:
        ss << "r" << inst.resIdx << ", r" << inst.leftIdx << ", r" << inst.rightIdx; break;
    case OpCode::JMP:          ss << "-> pc=" << inst.resIdx; break;
    case OpCode::JMP_REL:      ss << "-> pc+=" << (int32_t)inst.resIdx; break;
    case OpCode::JMP_IF_FALSE: ss << "r" << inst.resIdx << ", -> pc=" << inst.leftIdx; break;
    case OpCode::JMP_IF_TRUE:  ss << "r" << inst.resIdx << ", -> pc+=" << (int32_t)inst.leftIdx; break;
    case OpCode::BR_GT: case OpCode::BR_GT_U:
    case OpCode::BR_LT: case OpCode::BR_LT_U:
    case OpCode::BR_EQ:
    case OpCode::BR_GTE: case OpCode::BR_GTE_U:
    case OpCode::BR_LTE: case OpCode::BR_LTE_U:
        ss << "r" << inst.resIdx << ", r" << inst.leftIdx << ", offset=" << (int32_t)inst.rightIdx; break;
    case OpCode::CALL:
        ss << "r" << inst.resIdx << ", "
           << (inst.leftIdx < funcTable.size() ? funcTable[inst.leftIdx].name : "?")
           << "(r" << inst.rightIdx << ")"; break;
    case OpCode::READ_INT:    ss << "r" << inst.resIdx; break;
    case OpCode::READ_DOUBLE: ss << "r" << inst.resIdx; break;
    case OpCode::RET:
    case OpCode::HALT:
        ss << "r" << inst.resIdx; break;
    }
    return ss.str();
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
    for (int i = 0; i < (int)prog.size(); i++)
        cout << formatInst(i, prog[i], constPool, revVar) << "\n";
}

void runDebugger(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    vector<double>&            varValues,
    const map<string, int>&    varMap
) {
    map<int, string> revVar;
    for (auto& [name, id] : varMap) revVar[id] = name;

    double regs[NUM_REGS] = {};
    set<int> breakpoints;
    int pc = 0;
    bool stepping = true;

    cout << "\n[Debugger] Commands: s=step  c=continue  b <n>=breakpoint  d <n>=delete bp\n";
    cout << "           r=registers  v=variables  l=listing  q=quit\n\n";

    auto printRegs = [&]() {
        cout << "Registers:\n";
        for (int i = 0; i < NUM_REGS; i++) {
            if (regs[i] != 0.0)
                cout << "  r" << setw(2) << i << " = " << regs[i] << "\n";
        }
    };

    auto printVars = [&]() {
        cout << "Variables:\n";
        for (auto& [name, id] : varMap) {
            if (id < (int)varValues.size())
                cout << "  " << name << " = " << varValues[id] << "\n";
        }
    };

    auto printListing = [&]() {
        for (int i = 0; i < (int)prog.size(); i++) {
            bool bp = breakpoints.count(i);
            bool cur = (i == pc);
            cout << (bp ? "B" : " ") << (cur ? ">" : " ")
                 << formatInst(i, prog[i], constPool, revVar) << "\n";
        }
    };

    while (pc < (int)prog.size()) {
        bool atBreak = breakpoints.count(pc) && !stepping;
        if (stepping || atBreak) {
            if (atBreak) {
                stepping = true;
                cout << "[Breakpoint " << pc << "]\n";
            }
            cout << "  " << formatInst(pc, prog[pc], constPool, revVar) << "\n";
            cout << "dbg> ";
            string cmd;
            getline(cin, cmd);
            if (cmd == "q") break;
            if (cmd == "s" || cmd.empty()) {
                stepping = true;
            } else if (cmd == "c") {
                stepping = false;
            } else if (cmd == "r") {
                printRegs(); continue;
            } else if (cmd == "v") {
                printVars(); continue;
            } else if (cmd == "l") {
                printListing(); continue;
            } else if (cmd.size() >= 2 && cmd[0] == 'b' && cmd[1] == ' ') {
                int n = stoi(cmd.substr(2));
                breakpoints.insert(n);
                cout << "Breakpoint set at " << n << "\n";
                continue;
            } else if (cmd.size() >= 2 && cmd[0] == 'd' && cmd[1] == ' ') {
                int n = stoi(cmd.substr(2));
                breakpoints.erase(n);
                cout << "Breakpoint " << n << " removed\n";
                continue;
            }
        }

        const auto& inst = prog[pc++];
        switch ((OpCode)inst.op) {
        case OpCode::MOV_CONST:    regs[inst.resIdx] = constPool[inst.leftIdx]; break;
        case OpCode::LOAD_VAR:     regs[inst.resIdx] = varValues[inst.leftIdx]; break;
        case OpCode::STORE_VAR:
            while ((int)varValues.size() <= (int)inst.leftIdx) varValues.push_back(0.0);
            varValues[inst.leftIdx] = regs[inst.resIdx];
            break;
        case OpCode::ADD:      regs[inst.resIdx] = regs[inst.leftIdx] + regs[inst.rightIdx]; break;
        case OpCode::SUB:      regs[inst.resIdx] = regs[inst.leftIdx] - regs[inst.rightIdx]; break;
        case OpCode::MUL:      regs[inst.resIdx] = regs[inst.leftIdx] * regs[inst.rightIdx]; break;
        case OpCode::DIV:      regs[inst.resIdx] = regs[inst.rightIdx] != 0
                                   ? regs[inst.leftIdx] / regs[inst.rightIdx] : 0; break;
        case OpCode::MOD:      regs[inst.resIdx] = fmod(regs[inst.leftIdx], regs[inst.rightIdx]); break;
        case OpCode::BIT_AND:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] & (int64_t)regs[inst.rightIdx]; break;
        case OpCode::BIT_OR:   regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] | (int64_t)regs[inst.rightIdx]; break;
        case OpCode::BIT_XOR:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] ^ (int64_t)regs[inst.rightIdx]; break;
        case OpCode::SHL:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] << (int64_t)regs[inst.rightIdx]; break;
        case OpCode::SHR:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] >> (int64_t)regs[inst.rightIdx]; break;
        case OpCode::CMP_EQ:   regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ:  regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:   regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:   regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LTE:  regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GTE:  regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::JMP:      pc = inst.resIdx; break;
        case OpCode::JMP_REL:  pc += (int32_t)inst.resIdx; break;
        case OpCode::JMP_IF_FALSE: if (regs[inst.resIdx] == 0.0) pc = inst.leftIdx; break;
        case OpCode::JMP_IF_TRUE:  if (regs[inst.resIdx] != 0.0) pc += (int32_t)inst.leftIdx; break;
        case OpCode::BR_GT:    if (regs[inst.resIdx] >  regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT:    if (regs[inst.resIdx] <  regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_EQ:    if (regs[inst.resIdx] == regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE:   if (regs[inst.resIdx] >= regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE:   if (regs[inst.resIdx] <= regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GT_U:  if ((uint32_t)regs[inst.resIdx] >  (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT_U:  if ((uint32_t)regs[inst.resIdx] <  (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE_U: if ((uint32_t)regs[inst.resIdx] >= (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE_U: if ((uint32_t)regs[inst.resIdx] <= (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::READ_INT: { int v; cin >> v; regs[inst.resIdx] = v; break; }
        case OpCode::READ_DOUBLE: { double v; cin >> v; regs[inst.resIdx] = v; break; }
        case OpCode::CALL: {
            auto& fe = funcTable[inst.leftIdx];
            vector<double> args;
            for (int i = 0; i < (int)fe.params.size(); i++)
                args.push_back(regs[inst.rightIdx + i]);
            regs[inst.resIdx] = callFunction(inst.leftIdx, args);
            break;
        }
        case OpCode::RET:
        case OpCode::HALT:
            cout << "[VM] Result = " << regs[inst.resIdx] << "\n";
            return;
        }
    }
}

double executeWithMemory(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    Memory&                    memory,
    int                        startPC
) {
    uint32_t regs[NUM_REGS] = {};
    uint32_t IR = 0;             
    int pc = startPC;

    while (pc < (int)prog.size()) {

        IR = memory.read32(memory.codeRegion.base + pc * 4);
        const Instruction& inst = *reinterpret_cast<const Instruction*>(&IR);
        pc++;
        switch ((OpCode)inst.op) {
        case OpCode::MOV_CONST:
            regs[inst.resIdx] = constPool[inst.leftIdx];
            break;
        case OpCode::LOAD_VAR: {
            uint64_t addr = memory.dataRegion.base + inst.leftIdx * sizeof(double);
            regs[inst.resIdx] = memory.readDouble(addr);
            break;
        }
        case OpCode::STORE_VAR: {
            uint64_t addr = memory.dataRegion.base + inst.leftIdx * sizeof(double);
            memory.writeDouble(addr, regs[inst.resIdx]);
            break;
        }
        case OpCode::ADD:      regs[inst.resIdx] = regs[inst.leftIdx] + regs[inst.rightIdx]; break;
        case OpCode::SUB:      regs[inst.resIdx] = regs[inst.leftIdx] - regs[inst.rightIdx]; break;
        case OpCode::MUL:      regs[inst.resIdx] = regs[inst.leftIdx] * regs[inst.rightIdx]; break;
        case OpCode::DIV:      regs[inst.resIdx] = regs[inst.rightIdx] != 0
                                   ? regs[inst.leftIdx] / regs[inst.rightIdx] : 0; break;
        case OpCode::MOD:      regs[inst.resIdx] = fmod(regs[inst.leftIdx], regs[inst.rightIdx]); break;
        case OpCode::BIT_AND:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] & (int64_t)regs[inst.rightIdx]; break;
        case OpCode::BIT_OR:   regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] | (int64_t)regs[inst.rightIdx]; break;
        case OpCode::BIT_XOR:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] ^ (int64_t)regs[inst.rightIdx]; break;
        case OpCode::SHL:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] << (int64_t)regs[inst.rightIdx]; break;
        case OpCode::SHR:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] >> (int64_t)regs[inst.rightIdx]; break;
        case OpCode::CMP_EQ:   regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ:  regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:   regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:   regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LTE:  regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GTE:  regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::JMP:      pc = inst.resIdx; break;
        case OpCode::JMP_REL:  pc += (int32_t)inst.resIdx; break;
        case OpCode::JMP_IF_FALSE: if (regs[inst.resIdx] == 0.0) pc = inst.leftIdx; break;
        case OpCode::JMP_IF_TRUE:  if (regs[inst.resIdx] != 0.0) pc += (int32_t)inst.leftIdx; break;
        case OpCode::BR_GT:    if (regs[inst.resIdx] >  regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT:    if (regs[inst.resIdx] <  regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_EQ:    if (regs[inst.resIdx] == regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE:   if (regs[inst.resIdx] >= regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE:   if (regs[inst.resIdx] <= regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GT_U:  if ((uint32_t)regs[inst.resIdx] >  (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT_U:  if ((uint32_t)regs[inst.resIdx] <  (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE_U: if ((uint32_t)regs[inst.resIdx] >= (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE_U: if ((uint32_t)regs[inst.resIdx] <= (uint32_t)regs[inst.leftIdx]) pc += (int32_t)inst.rightIdx; break;
        case OpCode::READ_INT:    { int v; cin >> v; regs[inst.resIdx] = v; break; }
        case OpCode::READ_DOUBLE: { double v; cin >> v; regs[inst.resIdx] = v; break; }
        case OpCode::CALL: {
            auto& fe = funcTable[inst.leftIdx];
            vector<double> args;
            for (int i = 0; i < (int)fe.params.size(); i++)
                args.push_back(regs[inst.rightIdx + i]);
            regs[inst.resIdx] = callFunction(inst.leftIdx, args);
            break;
        }
        case OpCode::RET:
        case OpCode::HALT:
            return regs[inst.resIdx];
        }
    }
    return 0;
}
