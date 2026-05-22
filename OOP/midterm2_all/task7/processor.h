#pragma once
#include "instructions.h"
#include "memory.h"
#include <vector>
#include <cstdint>

static constexpr int PROC_NUM_REGS = 16;

struct Processor {
    double   reg[PROC_NUM_REGS] = {};
    uint64_t ip = 0;
    uint64_t sp = 0;
    uint64_t bp = 0;

    void init(Memory& mem, uint32_t entryPoint = 0) {
        for (int i = 0; i < PROC_NUM_REGS; i++) reg[i] = 0.0;
        ip = entryPoint;
        sp = mem.stackRegion.base + mem.stackRegion.size;
        bp = sp;
        mem.sp = sp;
    }

    double run(
        const std::vector<Instruction>& code,
        const std::vector<double>&      constPool,
        Memory&                         mem,
        const std::vector<uint32_t>&    jumpTable
    );
};
