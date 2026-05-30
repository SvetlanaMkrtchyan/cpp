#include "processor.h"
#include "executor.h"

double Processor::run(
    const std::vector<Instruction>& code,
    const std::vector<double>&      constPool,
    Memory&                         mem,
    const std::vector<uint32_t>&    jumpTable
) {
    (void)jumpTable;
    init(mem, 0);
    debugMode = false;
    runMode = ProcRunMode::RUN;
    return executeFlat(code, constPool, mem, jumpTable, (uint32_t)ip);
}

VMRunResult Processor::runDebug(const std::set<uint32_t>& breakpoints) {
    if (!vm) return VMRunResult::HALTED;
    debugMode = true;
    runMode = ProcRunMode::RUN_DEBUG;
    ip = vm->ip;
    VMRunResult r = vm->runUntil(breakpoints);
    ip = vm->ip;
    return r;
}

VMRunResult Processor::runSingleInt(StepMode mode) {
    if (!vm) return VMRunResult::HALTED;
    debugMode = true;
    switch (mode) {
    case StepMode::STEP_IN:  runMode = ProcRunMode::STEP_IN;  break;
    case StepMode::STEP_OVER: runMode = ProcRunMode::STEP_OVER; break;
    case StepMode::STEP_OUT: runMode = ProcRunMode::STEP_OUT; break;
    }
    ip = vm->ip;
    VMRunResult r = vm->runOne(mode);
    ip = vm->ip;
    return r;
}
