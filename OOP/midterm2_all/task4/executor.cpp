#include "executor.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <set>
#include <cstring>
#include <stdexcept>

using namespace std;

static const char* kRvAbiNames[NUM_REGS] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0",   "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6",   "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8",   "s9", "s10","s11","t3", "t4", "t5", "t6"
};

static Instruction decodeInstruction(uint32_t rawIR) {
    Instruction inst{};
    memcpy(&inst, &rawIR, sizeof(inst));
    return inst;
}

struct VmFlags {
    bool zf = false;
    bool cf = false;
    bool sf = false;
    bool of = false;
};

static void updateFlagsFromCompare(double lhs, double rhs, VmFlags& flags) {
    const int64_t sL = (int64_t)lhs;
    const int64_t sR = (int64_t)rhs;
    const uint64_t uL = (uint64_t)(uint32_t)lhs;
    const uint64_t uR = (uint64_t)(uint32_t)rhs;

    flags.zf = (lhs == rhs);
    flags.cf = (uL < uR);
    flags.sf = (sL < sR);
    const int64_t diff = sL - sR;
    flags.of = ((sL ^ sR) & (sL ^ diff)) < 0;
}

static bool shouldTakeFlagsJump(OpCode op, const VmFlags& flags) {
    switch (op) {
    case OpCode::JA:  return !flags.cf && !flags.zf;
    case OpCode::JAE: return !flags.cf;
    case OpCode::JB:  return flags.cf;
    case OpCode::JBE: return flags.cf || flags.zf;
    case OpCode::JE:  return flags.zf;
    case OpCode::JNE: return !flags.zf;
    default:          return false;
    }
}

static inline void enforceZeroRegister(double* regs) {
    regs[0] = 0.0;
}

double execute(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    vector<double>&            varValues,
    double*                    regs,
    int                        startPC
) {
    uint32_t IR = 0;
    int ip = startPC;
    VmFlags flags{};
    enforceZeroRegister(regs);
    while (ip < (int)prog.size()) {
        memcpy(&IR, &prog[ip], sizeof(IR));
        Instruction inst = decodeInstruction(IR);
        ip++;
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
        case OpCode::CMP_EQ:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_FLAGS: updateFlagsFromCompare(regs[inst.resIdx], regs[inst.leftIdx], flags); break;
        case OpCode::JMP:      ip = inst.resIdx; break;
        case OpCode::JMP_REL:  ip += (int32_t)inst.resIdx; break;
        case OpCode::JMP_IF_FALSE: if (regs[inst.resIdx] == 0.0) ip = inst.leftIdx; break;
        case OpCode::JMP_IF_TRUE:  if (regs[inst.resIdx] != 0.0) ip += (int32_t)inst.leftIdx; break;
        case OpCode::BR_GT:    if (regs[inst.resIdx] >  regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT:    if (regs[inst.resIdx] <  regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_EQ:    if (regs[inst.resIdx] == regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE:   if (regs[inst.resIdx] >= regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE:   if (regs[inst.resIdx] <= regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GT_U:  if ((uint32_t)regs[inst.resIdx] >  (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT_U:  if ((uint32_t)regs[inst.resIdx] <  (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE_U: if ((uint32_t)regs[inst.resIdx] >= (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE_U: if ((uint32_t)regs[inst.resIdx] <= (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::JA:
        case OpCode::JAE:
        case OpCode::JB:
        case OpCode::JBE:
        case OpCode::JE:
        case OpCode::JNE:
            if (shouldTakeFlagsJump((OpCode)inst.op, flags)) ip += (int8_t)inst.rightIdx;
            break;
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
        enforceZeroRegister(regs);
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
    case OpCode::CMP_FLAGS:
        ss << "cmp r" << inst.resIdx << ", r" << inst.leftIdx; break;
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
    case OpCode::JA: case OpCode::JAE: case OpCode::JB:
    case OpCode::JBE: case OpCode::JE: case OpCode::JNE:
        ss << "offset=" << (int)(int8_t)inst.rightIdx; break;
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
    VmFlags flags{};

    cout << "\n[Debugger] Commands: s=step  c=continue  b <n>=breakpoint  d <n>=delete bp\n";
    cout << "           r=registers  v=variables  l=listing  q=quit\n\n";

    auto printRegs = [&]() {
        cout << "Registers:\n";
        for (int i = 0; i < NUM_REGS; i++) {
            if (regs[i] != 0.0)
                cout << "  x" << setw(2) << i << " (" << setw(4) << kRvAbiNames[i] << ") = " << regs[i] << "\n";
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
        case OpCode::CMP_EQ:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_FLAGS: updateFlagsFromCompare(regs[inst.resIdx], regs[inst.leftIdx], flags); break;
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
        case OpCode::JA:
        case OpCode::JAE:
        case OpCode::JB:
        case OpCode::JBE:
        case OpCode::JE:
        case OpCode::JNE:
            if (shouldTakeFlagsJump((OpCode)inst.op, flags)) pc += (int8_t)inst.rightIdx;
            break;
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
        enforceZeroRegister(regs);
    }
}

double executeWithMemory(
    const vector<Instruction>& prog,
    const vector<double>&      constPool,
    Memory&                    memory,
    int                        startPC
) {
    double regs[NUM_REGS] = {};
    uint32_t IR = 0;
    int ip = startPC;
    VmFlags flags{};
    enforceZeroRegister(regs);

    while (ip < (int)prog.size()) {
        IR = memory.read32(memory.codeRegion.base + ip * 4);
        Instruction inst = decodeInstruction(IR);
        ip++;
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
        case OpCode::CMP_EQ:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_NEQ:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_LTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_GTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], flags); regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
        case OpCode::CMP_FLAGS: updateFlagsFromCompare(regs[inst.resIdx], regs[inst.leftIdx], flags); break;
        case OpCode::JMP:      ip = inst.resIdx; break;
        case OpCode::JMP_REL:  ip += (int32_t)inst.resIdx; break;
        case OpCode::JMP_IF_FALSE: if (regs[inst.resIdx] == 0.0) ip = inst.leftIdx; break;
        case OpCode::JMP_IF_TRUE:  if (regs[inst.resIdx] != 0.0) ip += (int32_t)inst.leftIdx; break;
        case OpCode::BR_GT:    if (regs[inst.resIdx] >  regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT:    if (regs[inst.resIdx] <  regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_EQ:    if (regs[inst.resIdx] == regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE:   if (regs[inst.resIdx] >= regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE:   if (regs[inst.resIdx] <= regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GT_U:  if ((uint32_t)regs[inst.resIdx] >  (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LT_U:  if ((uint32_t)regs[inst.resIdx] <  (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_GTE_U: if ((uint32_t)regs[inst.resIdx] >= (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::BR_LTE_U: if ((uint32_t)regs[inst.resIdx] <= (uint32_t)regs[inst.leftIdx]) ip += (int32_t)inst.rightIdx; break;
        case OpCode::JA:
        case OpCode::JAE:
        case OpCode::JB:
        case OpCode::JBE:
        case OpCode::JE:
        case OpCode::JNE:
            if (shouldTakeFlagsJump((OpCode)inst.op, flags)) ip += (int8_t)inst.rightIdx;
            break;
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
        enforceZeroRegister(regs);
    }
    return 0;
}

double executeRV32I(
    const vector<uint32_t>& rawProgram,
    Memory&                 memory,
    uint32_t                startPC
) {
    int32_t regs[32] = {};
    uint32_t pc = startPC;

    auto signExtend = [](uint32_t value, int bits) -> int32_t {
        const uint32_t mask = 1u << (bits - 1);
        value = value & ((1u << bits) - 1u);
        return (int32_t)((value ^ mask) - mask);
    };

    auto loadProgram = [&]() {
        for (size_t i = 0; i < rawProgram.size(); i++) {
            uint64_t addr = memory.codeRegion.base + i * 4;
            memory.write32(addr, rawProgram[i]);
        }
    };

    loadProgram();

    while ((pc / 4) < rawProgram.size()) {
        uint32_t inst = memory.read32(memory.codeRegion.base + pc);
        uint32_t opcode = inst & 0x7F;
        uint32_t rd     = (inst >> 7) & 0x1F;
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1    = (inst >> 15) & 0x1F;
        uint32_t rs2    = (inst >> 20) & 0x1F;
        uint32_t funct7 = (inst >> 25) & 0x7F;

        uint32_t nextPC = pc + 4;

        switch (opcode) {
        case 0x33: {
            switch (funct3) {
            case 0x0: regs[rd] = (funct7 == 0x20) ? (regs[rs1] - regs[rs2]) : (regs[rs1] + regs[rs2]); break;
            case 0x1: regs[rd] = regs[rs1] << (regs[rs2] & 0x1F); break;
            case 0x2: regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0; break;
            case 0x3: regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]) ? 1 : 0; break;
            case 0x4: regs[rd] = regs[rs1] ^ regs[rs2]; break;
            case 0x5: regs[rd] = (funct7 == 0x20) ? (regs[rs1] >> (regs[rs2] & 0x1F))
                                                  : (int32_t)((uint32_t)regs[rs1] >> (regs[rs2] & 0x1F)); break;
            case 0x6: regs[rd] = regs[rs1] | regs[rs2]; break;
            case 0x7: regs[rd] = regs[rs1] & regs[rs2]; break;
            default: throw runtime_error("Unsupported RV32I OP funct3");
            }
            break;
        }
        case 0x13: {
            int32_t imm = signExtend(inst >> 20, 12);
            switch (funct3) {
            case 0x0: regs[rd] = regs[rs1] + imm; break;
            case 0x2: regs[rd] = (regs[rs1] < imm) ? 1 : 0; break;
            case 0x3: regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)imm) ? 1 : 0; break;
            case 0x4: regs[rd] = regs[rs1] ^ imm; break;
            case 0x6: regs[rd] = regs[rs1] | imm; break;
            case 0x7: regs[rd] = regs[rs1] & imm; break;
            case 0x1: regs[rd] = regs[rs1] << ((inst >> 20) & 0x1F); break;
            case 0x5:
                if (((inst >> 30) & 0x1) == 1) regs[rd] = regs[rs1] >> ((inst >> 20) & 0x1F);
                else regs[rd] = (int32_t)((uint32_t)regs[rs1] >> ((inst >> 20) & 0x1F));
                break;
            default: throw runtime_error("Unsupported RV32I OP-IMM funct3");
            }
            break;
        }
        case 0x03: {
            int32_t imm = signExtend(inst >> 20, 12);
            uint64_t addr = (uint64_t)((int64_t)regs[rs1] + imm);
            switch (funct3) {
            case 0x0: regs[rd] = signExtend(memory.read8(addr), 8); break;
            case 0x1: regs[rd] = signExtend(memory.read16(addr), 16); break;
            case 0x2: regs[rd] = (int32_t)memory.read32(addr); break;
            case 0x4: regs[rd] = memory.read8(addr); break;
            case 0x5: regs[rd] = memory.read16(addr); break;
            default: throw runtime_error("Unsupported RV32I LOAD funct3");
            }
            break;
        }
        case 0x23: {
            uint32_t imm12 = ((inst >> 7) & 0x1F) | (((inst >> 25) & 0x7F) << 5);
            int32_t imm = signExtend(imm12, 12);
            uint64_t addr = (uint64_t)((int64_t)regs[rs1] + imm);
            switch (funct3) {
            case 0x0: memory.write8(addr, (uint8_t)regs[rs2]); break;
            case 0x1: memory.write16(addr, (uint16_t)regs[rs2]); break;
            case 0x2: memory.write32(addr, (uint32_t)regs[rs2]); break;
            default: throw runtime_error("Unsupported RV32I STORE funct3");
            }
            break;
        }
        case 0x63: {
            uint32_t imm13 = (((inst >> 31) & 0x1) << 12)
                           | (((inst >> 7)  & 0x1) << 11)
                           | (((inst >> 25) & 0x3F) << 5)
                           | (((inst >> 8)  & 0xF) << 1);
            int32_t imm = signExtend(imm13, 13);
            bool take = false;
            switch (funct3) {
            case 0x0: take = (regs[rs1] == regs[rs2]); break;
            case 0x1: take = (regs[rs1] != regs[rs2]); break;
            case 0x4: take = (regs[rs1] < regs[rs2]); break;
            case 0x5: take = (regs[rs1] >= regs[rs2]); break;
            case 0x6: take = ((uint32_t)regs[rs1] <  (uint32_t)regs[rs2]); break;
            case 0x7: take = ((uint32_t)regs[rs1] >= (uint32_t)regs[rs2]); break;
            default: throw runtime_error("Unsupported RV32I BRANCH funct3");
            }
            if (take) nextPC = pc + imm;
            break;
        }
        case 0x6F: {
            uint32_t imm21 = (((inst >> 31) & 0x1) << 20)
                           | (((inst >> 12) & 0xFF) << 12)
                           | (((inst >> 20) & 0x1) << 11)
                           | (((inst >> 21) & 0x3FF) << 1);
            int32_t imm = signExtend(imm21, 21);
            regs[rd] = (int32_t)(pc + 4);
            nextPC = pc + imm;
            break;
        }
        case 0x67: {
            int32_t imm = signExtend(inst >> 20, 12);
            uint32_t target = ((uint32_t)(regs[rs1] + imm)) & ~1u;
            regs[rd] = (int32_t)(pc + 4);
            nextPC = target;
            break;
        }
        case 0x37: {
            regs[rd] = (int32_t)(inst & 0xFFFFF000);
            break;
        }
        case 0x17: {
            regs[rd] = (int32_t)(pc + (inst & 0xFFFFF000));
            break;
        }
        case 0x73: {
            if (inst == 0x00000073) {
                return regs[10];
            }
            throw runtime_error("Unsupported RV32I SYSTEM instruction");
        }
        default:
            throw runtime_error("Unsupported RV32I opcode");
        }

        regs[0] = 0;
        pc = nextPC;
    }

    return regs[10];
}
