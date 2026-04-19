#pragma once
#include <vector>
#include <string>
#include <map>
#include <cstdint>

using namespace std;

static constexpr int NUM_REGS = 32;

enum class OpCode : uint32_t {
    MOV_CONST,
    LOAD_VAR,
    STORE_VAR,
    ADD, SUB, MUL, DIV, MOD,
    BIT_AND, BIT_OR, BIT_XOR, SHL, SHR,
    CMP_EQ, CMP_NEQ, CMP_LT, CMP_GT, CMP_LTE, CMP_GTE,
    JMP,
    JMP_REL,
    JMP_IF_FALSE,
    JMP_IF_TRUE,
    BR_GT, BR_LT, BR_EQ, BR_GTE, BR_LTE,
    BR_GT_U, BR_LT_U, BR_GTE_U, BR_LTE_U,
    CALL,
    RET,
    HALT,
    READ_INT,
    READ_DOUBLE,
};

extern string opNames[];

struct Instruction {
    uint32_t op       : 6;
    uint32_t resIdx   : 9;
    uint32_t leftIdx  : 9;
    uint32_t rightIdx : 8;
};
static_assert(sizeof(Instruction) == 4, "Instruction must be 4 bytes");

struct FuncEntry {
    string name;
    vector<string> params;
    vector<Instruction> code;
    vector<double> constPool;
};

extern vector<FuncEntry> funcTable;
extern map<string, int>  funcIndex;
