#pragma once
#include "instructions.h"
#include <string>
#include <vector>
#include <map>

static constexpr uint32_t EXEC_MAGIC   = 0x52564D00;
static constexpr uint32_t EXEC_VERSION = 1;

struct ExecHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t codeSize;
    uint32_t dataSize;
    uint32_t constSize;
    uint32_t varCount;
    uint32_t entryPoint;
};

struct ExecFile {
    ExecHeader           header;
    vector<Instruction>  code;
    vector<double>       constPool;
    vector<string>       varNames;
};

void saveExec(const string& path,
              const vector<Instruction>& code,
              const vector<double>&      constPool,
              const map<string, int>&    varMap,
              int                        entryPoint = 0);

ExecFile loadExec(const string& path);
