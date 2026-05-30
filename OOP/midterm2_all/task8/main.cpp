#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "executor.h"
#include "loader.h"
#include "memory.h"
#include "processor.h"
#include "debugger.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>

using namespace std;

static vector<Instruction> lastProg;
static vector<double>      lastConst;
static map<string, int>    lastVarMap;
static CompileInfo         lastInfo;

static void printHelp() {
    cout << "\nCommands:\n";
    cout << "  run     <code>   compile and run (shows assembly)\n";
    cout << "  runq    <code>   compile and run (quiet, no assembly dump)\n";
    cout << "  compile <code>   compile only, show stats\n";
    cout << "  runmem  <code>   compile and run with Memory model\n";
    cout << "  runstk  <code>   compile and run with stack frame (IA-32 style)\n";
    cout << "  debug   <code>   compile and debug step-by-step\n";
    cout << "  asm     <code>   compile and show disassembly\n";
    cout << "  meminfo          show Memory layout\n";
    cout << "  save    <file>   save last compiled program\n";
    cout << "  load    <file>   load and run exec file\n";
    cout << "  dbg     [file]   enter debugger CLI (load -f, go, step, br.add, print)\n";
    cout << "  xscan   <code>   run with x in [1..100] step 0.01\n";
    cout << "  rvrunhex <hex...> run raw RV32I 32-bit instructions (space-separated)\n";
    cout << "  help             show this help\n";
    cout << "  quit             exit\n\n";
    cout << "Language: int/double vars, if/else, while, for, do-while, switch,\n";
    cout << "          break/continue, print(expr), input(), functions.\n\n";
}

static void printVars(const map<string, int>& varMap, const vector<double>& varValues) {
    for (auto& [name, id] : varMap)
        if (id < (int)varValues.size())
            cout << "  " << name << " = " << varValues[id] << "\n";
}

static void printVarsMemory(const map<string, int>& varMap, Memory& memory) {
    for (auto& [name, id] : varMap) {
        uint64_t addr = memory.dataRegion.base + id * sizeof(double);
        cout << "  " << name << " = " << memory.readDouble(addr)
             << "  [0x" << hex << addr << dec << "]\n";
    }
}

static void appendHalt(vector<Instruction>& prog, int& nextReg, const CompileInfo& info) {
    int haltReg = info.finalReg;
    if (haltReg < 0 && info.lastVarId >= 0) {
        haltReg = nextReg++;
        prog.push_back({(uint32_t)OpCode::LOAD_VAR, (uint32_t)haltReg, (uint32_t)info.lastVarId, 0});
    }
    if (haltReg >= 0)
        prog.push_back({(uint32_t)OpCode::HALT, (uint32_t)haltReg, 0, 0});
}

static void compileInput(const string& input) {
    tokenList.clear();
    curIdx = 0;
    funcTable.clear();
    funcIndex.clear();
    lastProg.clear();
    lastConst.clear();
    lastVarMap.clear();
    lastInfo = {};

    tokenize(input);

    auto topSeq = make_unique<ExprNode>(NodeKind::SEQUENCE_NODE, "top");
    while (!atEnd()) {
        auto s = parseStatement();
        if (s) topSeq->children.push_back(move(s));
    }

    int nextReg = 1;
    compile(topSeq.get(), lastProg, lastConst, nextReg, lastVarMap, nullptr, nullptr, &lastInfo);
    appendHalt(lastProg, nextReg, lastInfo);
}

int main() {
    cout << "Mini Compiler + VM  (64KB RAM, section-based .exec)\n";
    cout << "Type 'help' for commands.\n\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        if (line.empty()) continue;

        string cmd, rest;
        size_t sp = line.find(' ');
        if (sp != string::npos) { cmd = line.substr(0, sp); rest = line.substr(sp + 1); }
        else                    { cmd = line; }

        if (cmd == "quit" || cmd == "exit") break;
        if (cmd == "help") { printHelp(); continue; }

        try {
            if (cmd == "meminfo") {
                Memory m;
                m.dumpLayout();
            }
            else if (cmd == "run" || cmd == "runq") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                if (cmd == "run") {
                    dumpProgram(lastProg, lastConst, lastVarMap, "Assembly");
                    for (auto& fe : funcTable) {
                        map<string, int> pmap;
                        for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
                        dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
                    }
                }
                vector<double> varValues(max((int)lastVarMap.size() + 32, 64), 0.0);
                double regs[NUM_REGS] = {};
                double result = execute(lastProg, lastConst, varValues, regs, 0);
                cout << "Result = " << result << "\n";
                if (!lastVarMap.empty()) {
                    cout << "Variables:\n";
                    printVars(lastVarMap, varValues);
                }
            }
            else if (cmd == "compile") {
                compileInput(rest);
                cout << "Compiled: " << lastInfo.instrCount << " instructions, "
                     << lastConst.size() << " constants, "
                     << lastVarMap.size() << " variables, "
                     << funcTable.size() << " functions\n";
            }
            else if (cmd == "runmem") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                Memory memory;
                memory.dumpLayout();
                double result = executeWithMemory(lastProg, lastConst, memory, 0);
                cout << "Result = " << result << "\n";
                if (!lastVarMap.empty()) {
                    cout << "Variables (Data region):\n";
                    printVarsMemory(lastVarMap, memory);
                }
            }
            else if (cmd == "runstk") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                dumpProgram(lastProg, lastConst, lastVarMap, "Assembly (Stack Frame)");
                for (auto& fe : funcTable) {
                    map<string, int> pmap;
                    for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
                    dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
                }
                Memory memory;
                memory.dumpLayout();
                CpuState cpu;
                cpu.ip = 0;
                cpu.sp = memory.stackRegion.base + memory.stackRegion.size;
                cpu.bp = cpu.sp;
                double result = executeWithStack(lastProg, lastConst, memory, cpu);
                cout << "Result = " << result << "\n";
                cout << "CPU state:  IP=" << cpu.ip << "  SP=0x" << hex << cpu.sp
                     << "  BP=0x" << cpu.bp << dec << "\n";
                if (!lastVarMap.empty()) {
                    cout << "Variables (Data region):\n";
                    printVarsMemory(lastVarMap, memory);
                }
            }
            else if (cmd == "debug") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                dumpProgram(lastProg, lastConst, lastVarMap, "Program");
                vector<double> varValues(max((int)lastVarMap.size() + 32, 64), 0.0);
                runDebugger(lastProg, lastConst, varValues, lastVarMap);
            }
            else if (cmd == "asm") {
                compileInput(rest);
                dumpProgram(lastProg, lastConst, lastVarMap, "Main Program");
                for (auto& fe : funcTable) {
                    map<string, int> pmap;
                    for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
                    dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
                }
            }
            else if (cmd == "save") {
                if (lastProg.empty()) { cout << "Nothing compiled yet.\n"; continue; }
                saveExec(rest, lastProg, lastConst, lastVarMap);
                cout << "Saved to " << rest << "\n";
            }
            else if (cmd == "dbg" || cmd == "debugger") {
                runDebuggerCLI(rest);
            }
            else if (cmd == "load") {
                ExecFile ef = loadExec(rest);
                map<string, int> varMap;
                for (int i = 0; i < (int)ef.varNames.size(); i++) varMap[ef.varNames[i]] = i;

                cout << "\n=== Symbol Table ===\n";
                for (int i = 0; i < (int)ef.symbolTable.size(); i++)
                    cout << "  [" << i << "] " << ef.symbolTable[i].name
                         << "  params=" << ef.symbolTable[i].paramCount
                         << "  offset=" << ef.symbolTable[i].offset << "\n";

                cout << "\n=== Jump Table ===\n";
                for (int i = 0; i < (int)ef.jumpTable.size(); i++)
                    cout << "  [" << i << "] "
                         << (i < (int)ef.symbolTable.size() ? ef.symbolTable[i].name : "?")
                         << "_address = " << ef.jumpTable[i] << "\n";

                cout << "\n=== Reloc Table ===\n";
                for (auto& re : ef.relocTable)
                    cout << "  instr[" << re.instrIndex << "] -> func["
                         << re.funcIndex << "] (" << ef.symbolTable[re.funcIndex].name << ")"
                         << "  resolved_offset=" << ef.jumpTable[re.funcIndex] << "\n";

                Memory memory;
                memory.dumpLayout();
                Processor cpu;
                cpu.init(memory, ef.header.entryPoint);
                double result = cpu.run(ef.code, ef.constPool, memory, ef.jumpTable);
                cout << "Result = " << result << "\n";
                cout << "CPU: IP=" << cpu.ip << "  SP=0x" << hex << cpu.sp
                     << "  BP=0x" << cpu.bp << dec << "\n";
                if (!varMap.empty()) {
                    cout << "Variables:\n";
                    printVarsMemory(varMap, memory);
                }
            }
            else if (cmd == "xscan") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                dumpProgram(lastProg, lastConst, lastVarMap, "Assembly");
                for (auto& fe : funcTable) {
                    map<string, int> pmap;
                    for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
                    dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
                }
                auto it = lastVarMap.find("x");
                if (it == lastVarMap.end()) {
                    cout << "Variable x is not defined.\n";
                    continue;
                }
                int xId = it->second;
                for (int i = 0; i <= 9900; i++) {
                    double x = 1.0 + 0.01 * i;
                    vector<double> varValues(max((int)lastVarMap.size() + 32, 64), 0.0);
                    varValues[xId] = x;
                    double regs[NUM_REGS] = {};
                    double result = execute(lastProg, lastConst, varValues, regs, 0);
                    cout << fixed << setprecision(4) << x << " " << setprecision(6) << result << "\n";
                }
            }
            else if (cmd == "rvrunhex") {
                vector<uint32_t> rawProgram;
                stringstream ss(rest);
                string tok;
                while (ss >> tok) {
                    rawProgram.push_back((uint32_t)stoul(tok, nullptr, 0));
                }
                if (rawProgram.empty()) {
                    cout << "No RV32I instructions given.\n";
                    continue;
                }
                Memory memory;
                memory.dumpLayout();
                double result = executeRV32I(rawProgram, memory, 0);
                cout << "RV32I a0 (x10) = " << result << "\n";
            }
            else {
                cout << "Unknown command. Type 'help'.\n";
            }
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
