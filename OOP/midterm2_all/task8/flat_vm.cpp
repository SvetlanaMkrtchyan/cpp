#include "flat_vm.h"
#include "executor.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <stdexcept>

using namespace std;

static int32_t signExt(uint32_t val, int bits) {
    uint32_t mask = 1u << (bits - 1);
    val &= (1u << bits) - 1;
    if (val & mask)
        val |= ~((1u << bits) - 1);
    return (int32_t)val;
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

static VmFlags gFlags;

void FlatVM::init(
    const vector<Instruction>& prog,
    const vector<double>&      pool,
    Memory&                    mem,
    uint32_t                   entryPoint
) {
    code = &prog;
    constPool = &pool;
    memory = &mem;
    memset(regs, 0, sizeof(regs));
    ip = (int)entryPoint;
    halted = false;
    result = 0.0;
    callDepth = 0;
    stepOutTarget = -1;
    callStack.clear();
    pendingArgs.clear();
    gFlags = {};
}

bool FlatVM::atBreakpoint(const set<uint32_t>& bps) const {
    return bps.count((uint32_t)ip) > 0;
}

void FlatVM::printCurrentInst(const map<string, int>& varMap) const {
    if (!code || ip < 0 || ip >= (int)code->size()) {
        cout << "IP out of range\n";
        return;
    }
    map<int, string> revVar;
    for (auto& [name, id] : varMap) revVar[id] = name;
    cout << "IP = 0x" << hex << ip << dec << "  "
         << formatInst(ip, (*code)[ip], *constPool, revVar) << "\n";
}

VMRunResult FlatVM::stepOne(StepMode mode) {
    if (halted || !code || ip >= (int)code->size())
        return VMRunResult::HALTED;

    if (mode == StepMode::STEP_OUT) {
        if (stepOutTarget < 0)
            stepOutTarget = callDepth - 1;
        if (callDepth > stepOutTarget) {
            VMRunResult r = execInstruction();
            if (r != VMRunResult::RUNNING)
                return r;
            return VMRunResult::RUNNING;
        }
        stepOutTarget = -1;
        return VMRunResult::STEP_DONE;
    }

    if (mode == StepMode::STEP_OVER) {
        if (ip < (int)code->size() && (OpCode)(*code)[ip].op == OpCode::CALL) {
            int savedIP = ip;
            VMRunResult r = execInstruction();
            if (r == VMRunResult::HALTED)
                return r;
            while (!halted && ip < (int)code->size() && callDepth > 0) {
                r = execInstruction();
                if (r == VMRunResult::HALTED)
                    return r;
            }
            (void)savedIP;
            return VMRunResult::STEP_DONE;
        }
    }

    return execInstruction();
}

VMRunResult FlatVM::runOne(StepMode mode) {
    VMRunResult r = stepOne(mode);
    if (r == VMRunResult::RUNNING)
        return VMRunResult::STEP_DONE;
    return r;
}

VMRunResult FlatVM::runUntil(const set<uint32_t>& bps) {
    while (!halted && ip < (int)code->size()) {
        if (atBreakpoint(bps))
            return VMRunResult::BREAKPOINT;
        VMRunResult r = execInstruction();
        if (r != VMRunResult::RUNNING)
            return r;
    }
    return halted ? VMRunResult::HALTED : VMRunResult::RUNNING;
}

VMRunResult FlatVM::execInstruction() {
    if (!code || ip >= (int)code->size()) {
        halted = true;
        return VMRunResult::HALTED;
    }

    Instruction inst = (*code)[ip];
    ip++;

    switch ((OpCode)inst.op) {
    case OpCode::MOV_CONST:    regs[inst.resIdx] = (*constPool)[inst.leftIdx]; break;
    case OpCode::LOAD_VAR: {
        uint64_t addr = memory->dataRegion.base + inst.leftIdx * sizeof(double);
        regs[inst.resIdx] = memory->readDouble(addr);
        break;
    }
    case OpCode::STORE_VAR: {
        uint64_t addr = memory->dataRegion.base + inst.leftIdx * sizeof(double);
        memory->writeDouble(addr, regs[inst.resIdx]);
        break;
    }
    case OpCode::ADD:      regs[inst.resIdx] = regs[inst.leftIdx] + regs[inst.rightIdx]; break;
    case OpCode::SUB:      regs[inst.resIdx] = regs[inst.leftIdx] - regs[inst.rightIdx]; break;
    case OpCode::MUL:      regs[inst.resIdx] = regs[inst.leftIdx] * regs[inst.rightIdx]; break;
    case OpCode::DIV:      regs[inst.resIdx] = inst.rightIdx != 0 && regs[inst.rightIdx] != 0
                               ? regs[inst.leftIdx] / regs[inst.rightIdx] : 0; break;
    case OpCode::MOD:      regs[inst.resIdx] = fmod(regs[inst.leftIdx], regs[inst.rightIdx]); break;
    case OpCode::BIT_AND:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] & (int64_t)regs[inst.rightIdx]; break;
    case OpCode::BIT_OR:   regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] | (int64_t)regs[inst.rightIdx]; break;
    case OpCode::BIT_XOR:  regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] ^ (int64_t)regs[inst.rightIdx]; break;
    case OpCode::SHL:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] << (int64_t)regs[inst.rightIdx]; break;
    case OpCode::SHR:      regs[inst.resIdx] = (int64_t)regs[inst.leftIdx] >> (int64_t)regs[inst.rightIdx]; break;
    case OpCode::CMP_EQ:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], gFlags); regs[inst.resIdx] = regs[inst.leftIdx] == regs[inst.rightIdx] ? 1.0 : 0.0; break;
    case OpCode::CMP_NEQ:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], gFlags); regs[inst.resIdx] = regs[inst.leftIdx] != regs[inst.rightIdx] ? 1.0 : 0.0; break;
    case OpCode::CMP_LT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], gFlags); regs[inst.resIdx] = regs[inst.leftIdx] <  regs[inst.rightIdx] ? 1.0 : 0.0; break;
    case OpCode::CMP_GT:   updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], gFlags); regs[inst.resIdx] = regs[inst.leftIdx] >  regs[inst.rightIdx] ? 1.0 : 0.0; break;
    case OpCode::CMP_LTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], gFlags); regs[inst.resIdx] = regs[inst.leftIdx] <= regs[inst.rightIdx] ? 1.0 : 0.0; break;
    case OpCode::CMP_GTE:  updateFlagsFromCompare(regs[inst.leftIdx], regs[inst.rightIdx], gFlags); regs[inst.resIdx] = regs[inst.leftIdx] >= regs[inst.rightIdx] ? 1.0 : 0.0; break;
    case OpCode::CMP_FLAGS: updateFlagsFromCompare(regs[inst.resIdx], regs[inst.leftIdx], gFlags); break;
    case OpCode::JMP:          ip = inst.resIdx; break;
    case OpCode::JMP_REL:      ip += signExt(inst.resIdx, 9); break;
    case OpCode::JMP_IF_FALSE: if (regs[inst.resIdx] == 0.0) ip = inst.leftIdx; break;
    case OpCode::JMP_IF_TRUE:  if (regs[inst.resIdx] != 0.0) ip += signExt(inst.leftIdx, 9); break;
    case OpCode::BR_GT:    if (regs[inst.resIdx] >  regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_LT:    if (regs[inst.resIdx] <  regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_EQ:    if (regs[inst.resIdx] == regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_GTE:   if (regs[inst.resIdx] >= regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_LTE:   if (regs[inst.resIdx] <= regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_GT_U:  if ((uint32_t)regs[inst.resIdx] >  (uint32_t)regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_LT_U:  if ((uint32_t)regs[inst.resIdx] <  (uint32_t)regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_GTE_U: if ((uint32_t)regs[inst.resIdx] >= (uint32_t)regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::BR_LTE_U: if ((uint32_t)regs[inst.resIdx] <= (uint32_t)regs[inst.leftIdx]) ip += signExt(inst.rightIdx, 8); break;
    case OpCode::JA:
    case OpCode::JAE:
    case OpCode::JB:
    case OpCode::JBE:
    case OpCode::JE:
    case OpCode::JNE:
        if (shouldTakeFlagsJump((OpCode)inst.op, gFlags)) ip += (int8_t)inst.rightIdx;
        break;
    case OpCode::READ_INT:    { int v; cin >> v; regs[inst.resIdx] = v; break; }
    case OpCode::READ_DOUBLE: { double v; cin >> v; regs[inst.resIdx] = v; break; }
    case OpCode::PRINT:       cout << regs[inst.resIdx] << "\n"; break;
    case OpCode::CALL: {
        uint32_t targetOffset = inst.leftIdx;
        int      resReg       = (int)inst.resIdx;
        int      numArgs      = (int)pendingArgs.size();

        CallFrame frame;
        frame.retIP   = ip;
        frame.numArgs = numArgs;
        frame.resReg  = resReg;
        memcpy(frame.savedRegs, regs, sizeof(regs));

        double savedData[64] = {};
        for (int i = 0; i < numArgs && i < 64; i++) {
            uint64_t addr = memory->dataRegion.base + i * sizeof(double);
            savedData[i] = memory->readDouble(addr);
        }
        memcpy(frame.savedData, savedData, sizeof(savedData));
        callStack.push_back(frame);
        callDepth++;

        for (int i = 0; i < numArgs; i++) {
            uint64_t addr = memory->dataRegion.base + i * sizeof(double);
            memory->writeDouble(addr, pendingArgs[numArgs - 1 - i]);
        }
        pendingArgs.clear();

        regs[resReg] = 0.0;
        ip = (int)targetOffset;
        break;
    }
    case OpCode::RET: {
        if (callStack.empty()) {
            result = regs[inst.resIdx];
            halted = true;
            return VMRunResult::HALTED;
        }
        double retVal = regs[inst.resIdx];
        CallFrame frame = callStack.back();
        callStack.pop_back();
        callDepth--;
        int resReg = frame.resReg;
        for (int i = 0; i < frame.numArgs && i < 64; i++) {
            uint64_t addr = memory->dataRegion.base + i * sizeof(double);
            memory->writeDouble(addr, frame.savedData[i]);
        }
        memcpy(regs, frame.savedRegs, sizeof(regs));
        regs[resReg] = retVal;
        ip = frame.retIP;
        break;
    }
    case OpCode::PUSH_ARG:
        pendingArgs.push_back(regs[inst.resIdx]);
        break;
    case OpCode::PUSH_BP:
    case OpCode::MOV_BP_SP:
    case OpCode::POP_BP:
    case OpCode::SUB_SP:
        break;
    case OpCode::HALT:
        result = regs[inst.resIdx];
        halted = true;
        return VMRunResult::HALTED;
    default: break;
    }
    regs[0] = 0.0;
    return VMRunResult::RUNNING;
}
