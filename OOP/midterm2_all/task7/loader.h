#pragma once
#include "instructions.h"
#include <string>
#include <vector>
#include <map>

static constexpr uint32_t EXEC_MAGIC   = 0x52564D00;
static constexpr uint32_t EXEC_VERSION = 2;

static constexpr uint32_t SECTION_CODE   = 0;
static constexpr uint32_t SECTION_DATA   = 1;
static constexpr uint32_t SECTION_SYMTBL = 2;
static constexpr uint32_t SECTION_COUNT  = 3;

struct SectionHeader {
    uint32_t type;
    uint32_t size;
    uint32_t offset;
};

struct ExecHeader {
    uint32_t magic;
    uint32_t header_size;
    uint32_t section_count;
    uint32_t entryPoint;
    uint32_t varCount;
    uint32_t funcCount;
    uint32_t relocCount;
    SectionHeader sections[SECTION_COUNT];
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
