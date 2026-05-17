#pragma once
#include "instructions.h"
#include <string>
#include <vector>
#include <map>

static constexpr uint32_t EXEC_MAGIC   = 0x52564D00;
static constexpr uint32_t EXEC_VERSION = 2;

struct ExecHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t codeSize;
    uint32_t dataSize;
    uint32_t constSize;
    uint32_t varCount;
    uint32_t entryPoint;
    uint32_t funcCount;
    uint32_t relocCount;
};

struct FuncSymbol {
    string   name;
    uint32_t offset;
    uint32_t paramCount;
};

struct RelocEntry {
    uint32_t instrIndex;
    uint32_t funcIndex;
};

struct ExecFile {
    ExecHeader          header;
    vector<Instruction> code;
    vector<double>      constPool;
    vector<string>      varNames;
    vector<FuncSymbol>  symbolTable;
    vector<RelocEntry>  relocTable;
    vector<uint32_t>    jumpTable;
};

void saveExec(const string& path,
              const vector<Instruction>& mainCode,
              const vector<double>&      mainConst,
              const map<string, int>&    varMap,
              int                        entryPoint = 0);

ExecFile loadExec(const string& path);
