#include "debugger.h"
#include "executor.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

using namespace std;

static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static string toLower(string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

DbgCommand parseDbgCommand(const string& line) {
    DbgCommand cmd;
    istringstream iss(line);
    string token;
    string verb;

    if (!(iss >> verb)) return cmd;
    verb = toLower(verb);

    if (verb == "load") {
        cmd.code = DbgCmdCode::LOAD;
    } else if (verb == "go") {
        cmd.code = DbgCmdCode::GO;
    } else if (verb == "step") {
        cmd.code = DbgCmdCode::STEP;
        string sub;
        if (iss >> sub) {
            sub = toLower(sub);
            if (sub == "in")       cmd.code = DbgCmdCode::STEP_IN;
            else if (sub == "out") cmd.code = DbgCmdCode::STEP_OUT;
            else if (sub == "over") cmd.code = DbgCmdCode::STEP;
            else cmd.positional.push_back(sub);
        }
    } else if (verb == "stepin" || verb == "step-in" || verb == "step_in") {
        cmd.code = DbgCmdCode::STEP_IN;
    } else if (verb == "stepout" || verb == "step-out" || verb == "step_out") {
        cmd.code = DbgCmdCode::STEP_OUT;
    } else if (verb == "trace") {
        cmd.code = DbgCmdCode::STEP;
    } else if (verb == "br.add" || verb == "br") {
        cmd.code = DbgCmdCode::BR_ADD;
    } else if (verb == "br.del" || verb == "br.rm") {
        cmd.code = DbgCmdCode::BR_DEL;
    } else if (verb == "br.list" || verb == "br.ls") {
        cmd.code = DbgCmdCode::BR_LIST;
    } else if (verb == "print") {
        cmd.code = DbgCmdCode::PRINT;
    } else if (verb == "quit" || verb == "exit" || verb == "q") {
        cmd.code = DbgCmdCode::QUIT;
    } else if (verb == "help" || verb == "?") {
        cmd.code = DbgCmdCode::HELP;
    }

    while (iss >> token) {
        if (token.size() >= 2 && token[0] == '-' && token[1] == '-') {
            string key = token.substr(2);
            string val;
            if (iss >> val) cmd.args[key] = val;
        } else if (token.size() >= 2 && token[0] == '-') {
            string key = token.substr(1);
            string val;
            if (iss >> val) cmd.args[key] = val;
            else cmd.args[key] = "true";
        } else {
            cmd.positional.push_back(token);
        }
    }

    return cmd;
}

void DebugSession::printHelp() const {
    cout << "\nDebugger commands (verb [args] -flag value):\n";
    cout << "  load -f <path>              load .exec file (check magic, read header)\n";
    cout << "  go                          run until breakpoint or halt\n";
    cout << "  step                        step over (one instruction, skip calls)\n";
    cout << "  step in                     step into function calls\n";
    cout << "  step out                    run until current function returns\n";
    cout << "  br.add -func <name> -offset <n> [-cnd <expr>]   add breakpoint\n";
    cout << "  br.add -offset <n>          breakpoint at absolute IP\n";
    cout << "  br.del -offset <n>          remove breakpoint\n";
    cout << "  br.list                     list breakpoints\n";
    cout << "  print -mode functions       show symbol table\n";
    cout << "  print -mode ip              show IP and current instruction\n";
    cout << "  print -mode vars            show variables\n";
    cout << "  print -mode regs            show registers\n";
    cout << "  help                        show this help\n";
    cout << "  quit                        exit debugger\n\n";
}

bool DebugSession::loadFile(const map<string, string>& args) {
    string path;
    if (args.count("f")) path = args.at("f");
    else if (args.count("file")) path = args.at("file");
    else if (!args.empty()) path = args.begin()->second;

    if (path.empty()) {
        cout << "Usage: load -f <path>\n";
        return false;
    }

    cout << "[1] Check file (magic byte)...\n";
    execFile_ = loadExec(path);
    loadedPath_ = path;
    cout << "[2] Read header: entry=0x" << hex << execFile_.header.entryPoint
         << " funcs=" << dec << execFile_.header.funcCount
         << " vars=" << execFile_.header.varCount << "\n";

    varMap_.clear();
    for (int i = 0; i < (int)execFile_.varNames.size(); i++)
        varMap_[execFile_.varNames[i]] = i;

    initExecEnv();
    loaded = true;
    cout << "[3] Init exec env OK. Ready to debug.\n\n";
    return true;
}

void DebugSession::initExecEnv() {
    memory_ = Memory();
    vm_.init(execFile_.code, execFile_.constPool, memory_, execFile_.header.entryPoint);
    cpu_.init(memory_, execFile_.header.entryPoint);
    cpu_.attachVM(&vm_);
    breakpoints_.clear();
}

void DebugSession::cleanup() {
    if (loaded && vm_.halted)
        cout << "[Cleanup] Program halted. Result = " << vm_.result << "\n";
}

uint32_t DebugSession::resolveBreakpointAddr(const string& funcName, uint32_t offset) const {
    if (!funcName.empty()) {
        for (auto& sym : execFile_.symbolTable) {
            if (sym.name == funcName)
                return sym.offset + offset;
        }
        throw runtime_error("Unknown function: " + funcName);
    }
    return offset;
}

void DebugSession::showState() const {
    cout << "  SP=0x" << hex << cpu_.sp << "  BP=0x" << cpu_.bp
         << "  IP=0x" << vm_.ip << dec;
    if (vm_.halted) cout << "  [HALTED]";
    cout << "\n";
    vm_.printCurrentInst(varMap_);
}

void DebugSession::cmdGo() {
    if (!loaded) { cout << "No program loaded. Use: load -f <path>\n"; return; }
    if (vm_.halted) { cout << "Program already halted.\n"; return; }

    VMRunResult r = cpu_.runDebug(breakpoints_);
    switch (r) {
    case VMRunResult::BREAKPOINT:
        cout << "[Breakpoint at IP=0x" << hex << vm_.ip << dec << "]\n";
        showState();
        break;
    case VMRunResult::HALTED:
        cout << "[Program halted] Result = " << vm_.result << "\n";
        break;
    default:
        showState();
        break;
    }
}

void DebugSession::cmdStep(StepMode mode) {
    if (!loaded) { cout << "No program loaded.\n"; return; }
    if (vm_.halted) { cout << "Program already halted.\n"; return; }

    if (vm_.atBreakpoint(breakpoints_))
        cout << "[At breakpoint IP=0x" << hex << vm_.ip << dec << "]\n";

    VMRunResult r = cpu_.runSingleInt(mode);
    switch (r) {
    case VMRunResult::HALTED:
        cout << "[Program halted] Result = " << vm_.result << "\n";
        break;
    case VMRunResult::BREAKPOINT:
        cout << "[Breakpoint]\n";
        break;
    default:
        break;
    }
    showState();
}

void DebugSession::cmdBrAdd(const DbgCommand& cmd) {
    if (!loaded) { cout << "No program loaded.\n"; return; }

    string funcName;
    uint32_t offset = 0;
    bool hasOffset = false;

    if (cmd.args.count("func"))  funcName = cmd.args.at("func");
    if (cmd.args.count("offset")) {
        offset = (uint32_t)stoul(cmd.args.at("offset"), nullptr, 0);
        hasOffset = true;
    }
    if (!hasOffset && cmd.args.count("ip"))
        offset = (uint32_t)stoul(cmd.args.at("ip"), nullptr, 0);

    if (!hasOffset && funcName.empty() && !cmd.positional.empty())
        offset = (uint32_t)stoul(cmd.positional[0], nullptr, 0);

    try {
        uint32_t addr = resolveBreakpointAddr(funcName, offset);
        breakpoints_.insert(addr);
        cout << "Breakpoint set at IP=0x" << hex << addr << dec;
        if (!funcName.empty())
            cout << " (" << funcName << "+" << offset << ")";
        if (cmd.args.count("cnd"))
            cout << "  condition=" << cmd.args.at("cnd") << " [not evaluated yet]";
        cout << "\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
    }
}

void DebugSession::cmdBrDel(const DbgCommand& cmd) {
    uint32_t offset = 0;
    if (cmd.args.count("offset"))
        offset = (uint32_t)stoul(cmd.args.at("offset"), nullptr, 0);
    else if (!cmd.positional.empty())
        offset = (uint32_t)stoul(cmd.positional[0], nullptr, 0);

    string funcName = cmd.args.count("func") ? cmd.args.at("func") : "";
    try {
        uint32_t addr = resolveBreakpointAddr(funcName, offset);
        breakpoints_.erase(addr);
        cout << "Breakpoint removed at IP=0x" << hex << addr << dec << "\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
    }
}

void DebugSession::cmdBrList() {
    if (breakpoints_.empty()) {
        cout << "No breakpoints set.\n";
        return;
    }
    cout << "Breakpoints:\n";
    for (uint32_t bp : breakpoints_) {
        cout << "  IP=0x" << hex << bp << dec;
        for (auto& sym : execFile_.symbolTable) {
            if (bp >= sym.offset) {
                uint32_t rel = bp - sym.offset;
                cout << "  (" << sym.name << "+" << rel << ")";
                break;
            }
        }
        cout << "\n";
    }
}

void DebugSession::cmdPrint(const DbgCommand& cmd) {
    if (!loaded) { cout << "No program loaded.\n"; return; }

    string mode = cmd.args.count("mode") ? toLower(cmd.args.at("mode")) : "";

    if (mode.empty() && !cmd.positional.empty())
        mode = toLower(cmd.positional[0]);

    if (mode == "functions" || mode == "func" || mode == "sym") {
        cout << "\n=== Symbol Table ===\n";
        for (int i = 0; i < (int)execFile_.symbolTable.size(); i++) {
            auto& sym = execFile_.symbolTable[i];
            cout << "  [" << i << "] " << sym.name
                 << "  offset=0x" << hex << sym.offset << dec
                 << "  params=" << sym.paramCount << "\n";
        }
        cout << "\n=== Jump Table ===\n";
        for (int i = 0; i < (int)execFile_.jumpTable.size(); i++)
            cout << "  [" << i << "] " << execFile_.symbolTable[i].name
                 << "_address = 0x" << hex << execFile_.jumpTable[i] << dec << "\n";
    }
    else if (mode == "ip" || mode == "instr") {
        cout << "IP = 0x" << hex << vm_.ip << dec << "\n";
        vm_.printCurrentInst(varMap_);
    }
    else if (mode == "vars" || mode == "variables") {
        cout << "Variables:\n";
        for (auto& [name, id] : varMap_) {
            uint64_t addr = memory_.dataRegion.base + id * sizeof(double);
            cout << "  " << name << " = " << memory_.readDouble(addr)
                 << "  [0x" << hex << addr << dec << "]\n";
        }
    }
    else if (mode == "regs" || mode == "registers") {
        cout << "Registers:\n";
        for (int i = 0; i < NUM_REGS; i++)
            if (vm_.regs[i] != 0.0)
                cout << "  r" << i << " = " << vm_.regs[i] << "\n";
        cout << "  IP=0x" << hex << vm_.ip << "  SP=0x" << cpu_.sp
             << "  BP=0x" << cpu_.bp << dec << "\n";
    }
    else if (mode == "code" || mode == "listing") {
        map<int, string> revVar;
        for (auto& [name, id] : varMap_) revVar[id] = name;
        for (int i = 0; i < (int)execFile_.code.size(); i++) {
            bool bp = breakpoints_.count(i);
            bool cur = (i == vm_.ip);
            cout << (bp ? "B" : " ") << (cur ? ">" : " ")
                 << formatInst(i, execFile_.code[i], execFile_.constPool, revVar) << "\n";
        }
    }
    else {
        cout << "Usage: print -mode <functions|ip|vars|regs|code>\n";
    }
}

void DebugSession::execCommand(const DbgCommand& cmd) {
    switch (cmd.code) {
    case DbgCmdCode::LOAD:
        loadFile(cmd.args);
        break;
    case DbgCmdCode::GO:
        cmdGo();
        break;
    case DbgCmdCode::STEP:
        cmdStep(StepMode::STEP_OVER);
        break;
    case DbgCmdCode::STEP_IN:
        cmdStep(StepMode::STEP_IN);
        break;
    case DbgCmdCode::STEP_OUT:
        cmdStep(StepMode::STEP_OUT);
        break;
    case DbgCmdCode::BR_ADD:
        cmdBrAdd(cmd);
        break;
    case DbgCmdCode::BR_DEL:
        cmdBrDel(cmd);
        break;
    case DbgCmdCode::BR_LIST:
        cmdBrList();
        break;
    case DbgCmdCode::PRINT:
        cmdPrint(cmd);
        break;
    case DbgCmdCode::HELP:
        printHelp();
        break;
    case DbgCmdCode::QUIT:
        break;
    default:
        cout << "Unknown command. Type 'help'.\n";
        break;
    }
}

void DebugSession::runLoop() {
    cout << "=== Debugger CLI ===\n";
    cout << "Syntax: verb [arg1] [arg2] -flag value\n";
    printHelp();

    string line;
    while (true) {
        cout << "dbg> ";
        if (!getline(cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;

        DbgCommand cmd = parseDbgCommand(line);
        if (cmd.code == DbgCmdCode::QUIT) break;
        if (cmd.code == DbgCmdCode::UNKNOWN) {
            cout << "Unknown command: " << line << "\n";
            continue;
        }

        try {
            execCommand(cmd);
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << "\n";
        }
    }
    cleanup();
}

void runDebuggerCLI(const string& initialFile) {
    DebugSession session;
    if (!initialFile.empty()) {
        map<string, string> args;
        args["f"] = initialFile;
        session.loadFile(args);
    }
    session.runLoop();
}
