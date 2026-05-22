#include "processor.h"
#include "executor.h"

double Processor::run(
    const std::vector<Instruction>& code,
    const std::vector<double>&      constPool,
    Memory&                         mem,
    const std::vector<uint32_t>&    jumpTable
) {
    init(mem, 0);
    return executeFlat(code, constPool, mem, jumpTable, (uint32_t)ip);

}
