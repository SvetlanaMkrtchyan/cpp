#pragma once
#include "loader.h"
#include "flat_vm.h"
#include "memory.h"
#include "processor.h"
#include <map>
#include <set>
#include <string>
#include <vector>

enum class DbgCmdCode {
    UNKNOWN,
    LOAD,
    GO,
    STEP,
    STEP_IN,
    STEP_OUT,
    BR_ADD,
    BR_DEL,
    BR_LIST,
    PRINT,
    QUIT,
    HELP
};

struct DbgCommand {
    DbgCmdCode                    code = DbgCmdCode::UNKNOWN;
    std::map<std::string, std::string> args;
    std::vector<std::string>      positional;
};

DbgCommand parseDbgCommand(const std::string& line);

class DebugSession {
public:
    bool loaded = false;

    bool loadFile(const std::map<std::string, std::string>& args);
    void runLoop();

private:
    ExecFile              execFile_;
    Memory                memory_;
    Processor             cpu_;
    FlatVM                vm_;
    std::map<std::string, int> varMap_;
    std::set<uint32_t>    breakpoints_;
    std::string           loadedPath_;

    void initExecEnv();
    void cleanup();

    void execCommand(const DbgCommand& cmd);
    void cmdGo();
    void cmdStep(StepMode mode);
    void cmdBrAdd(const DbgCommand& cmd);
    void cmdBrDel(const DbgCommand& cmd);
    void cmdBrList();
    void cmdPrint(const DbgCommand& cmd);
    void printHelp() const;
    void showState() const;

    uint32_t resolveBreakpointAddr(const std::string& funcName, uint32_t offset) const;
};

void runDebuggerCLI(const std::string& initialFile = "");
