#pragma once
#include "instructions.h"
#include "memory.h"
#include <vector>
#include <set>
#include <map>
#include <cstdint>

enum class StepMode {
    STEP_IN,
    STEP_OVER,
    STEP_OUT
};

enum class VMRunResult {
    RUNNING,
    HALTED,
    BREAKPOINT,
    STEP_DONE
};

struct FlatVM {
    const std::vector<Instruction>* code     = nullptr;
    const std::vector<double>*      constPool = nullptr;
    Memory*                         memory   = nullptr;

    double   regs[NUM_REGS] = {};
    int      ip             = 0;
    bool     halted         = false;
    double   result         = 0.0;
    int      callDepth      = 0;
    int      stepOutTarget  = -1;

    struct CallFrame {
        int    retIP;
        int    numArgs;
        int    resReg;
        double savedRegs[NUM_REGS];
        double savedData[64];
    };
    std::vector<CallFrame> callStack;
    std::vector<double>    pendingArgs;

    void init(
        const std::vector<Instruction>& prog,
        const std::vector<double>&      pool,
        Memory&                         mem,
        uint32_t                        entryPoint = 0
    );

    bool atBreakpoint(const std::set<uint32_t>& bps) const;
    void printCurrentInst(const std::map<std::string, int>& varMap) const;

    VMRunResult stepOne(StepMode mode);
    VMRunResult runUntil(const std::set<uint32_t>& bps);
    VMRunResult runOne(StepMode mode);

private:
    VMRunResult execInstruction();
};
