#include "loader.h"
#include <fstream>
#include <stdexcept>

using namespace std;

static void writeStr(ofstream& f, const string& s) {
    uint32_t len = s.size();
    f.write(reinterpret_cast<const char*>(&len), sizeof(len));
    f.write(s.data(), len);
}

static string readStr(ifstream& f) {
    uint32_t len;
    f.read(reinterpret_cast<char*>(&len), sizeof(len));
    string s(len, '\0');
    f.read(s.data(), len);
    return s;
}

void saveExec(const string& path,
              const vector<Instruction>& mainCode,
              const vector<double>&      mainConst,
              const map<string, int>&    varMap,
              int                        entryPoint)
{
    vector<Instruction> flatCode = mainCode;
    vector<double>      flatConst = mainConst;

    vector<FuncSymbol> symbolTable;
    vector<RelocEntry> relocTable;

    for (int fi = 0; fi < (int)funcTable.size(); fi++) {
        auto& fe = funcTable[fi];
        FuncSymbol sym;
        sym.name       = fe.name;
        sym.offset     = (uint32_t)flatCode.size();
        sym.paramCount = (uint32_t)fe.params.size();

        uint32_t constBase = (uint32_t)flatConst.size();
        for (double c : fe.constPool)
            flatConst.push_back(c);

        for (auto inst : fe.code) {
            if ((OpCode)inst.op == OpCode::MOV_CONST)
                inst.leftIdx += constBase;
            flatCode.push_back(inst);
        }

        symbolTable.push_back(sym);
    }

    for (int i = 0; i < (int)flatCode.size(); i++) {
        if ((OpCode)flatCode[i].op == OpCode::CALL) {
            RelocEntry re;
            re.instrIndex = (uint32_t)i;
            re.funcIndex  = (uint32_t)flatCode[i].leftIdx;
            relocTable.push_back(re);
        }
    }

    vector<string> varNames(varMap.size());
    for (auto& [name, id] : varMap) varNames[id] = name;

    uint32_t codeBytes  = (uint32_t)(flatCode.size() * sizeof(Instruction));
    uint32_t constBytes = (uint32_t)(flatConst.size() * sizeof(double));

    uint32_t hdrSize = (uint32_t)sizeof(ExecHeader);

    uint32_t codeOffset  = hdrSize;
    uint32_t constOffset = codeOffset + codeBytes;
    uint32_t symtblOffset = constOffset + constBytes;

    ExecHeader hdr;
    hdr.magic         = EXEC_MAGIC;
    hdr.header_size   = hdrSize;
    hdr.section_count = SECTION_COUNT;
    hdr.entryPoint    = (uint32_t)entryPoint;
    hdr.varCount      = (uint32_t)varNames.size();
    hdr.funcCount     = (uint32_t)symbolTable.size();
    hdr.relocCount    = (uint32_t)relocTable.size();

    hdr.sections[SECTION_CODE]   = { SECTION_CODE,   codeBytes,  codeOffset  };
    hdr.sections[SECTION_DATA]   = { SECTION_DATA,   constBytes, constOffset };
    hdr.sections[SECTION_SYMTBL] = { SECTION_SYMTBL, 0,          symtblOffset };

    ofstream f(path, ios::binary);
    if (!f) throw runtime_error("Cannot write: " + path);

    f.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    f.write(reinterpret_cast<const char*>(flatCode.data()),
            flatCode.size() * sizeof(Instruction));

    f.write(reinterpret_cast<const char*>(flatConst.data()),
            flatConst.size() * sizeof(double));

    for (auto& name : varNames)
        writeStr(f, name);

    for (auto& sym : symbolTable) {
        writeStr(f, sym.name);
        f.write(reinterpret_cast<const char*>(&sym.offset),     sizeof(uint32_t));
        f.write(reinterpret_cast<const char*>(&sym.paramCount), sizeof(uint32_t));
    }

    for (auto& re : relocTable) {
        f.write(reinterpret_cast<const char*>(&re.instrIndex), sizeof(uint32_t));
        f.write(reinterpret_cast<const char*>(&re.funcIndex),  sizeof(uint32_t));
    }
}

ExecFile loadExec(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) throw runtime_error("Cannot open: " + path);

    ExecFile ef;
    f.read(reinterpret_cast<char*>(&ef.header), sizeof(ExecHeader));

    if (ef.header.magic != EXEC_MAGIC)
        throw runtime_error("Invalid exec file magic");
    if (ef.header.section_count != SECTION_COUNT)
        throw runtime_error("Unsupported section count");

    uint32_t codeBytes  = ef.header.sections[SECTION_CODE].size;
    uint32_t constBytes = ef.header.sections[SECTION_DATA].size;

    ef.code.resize(codeBytes / sizeof(Instruction));
    f.seekg(ef.header.sections[SECTION_CODE].offset);
    f.read(reinterpret_cast<char*>(ef.code.data()), codeBytes);

    ef.constPool.resize(constBytes / sizeof(double));
    f.seekg(ef.header.sections[SECTION_DATA].offset);
    f.read(reinterpret_cast<char*>(ef.constPool.data()), constBytes);

    f.seekg(ef.header.sections[SECTION_SYMTBL].offset);

    ef.varNames.resize(ef.header.varCount);
    for (uint32_t i = 0; i < ef.header.varCount; i++)
        ef.varNames[i] = readStr(f);

    ef.symbolTable.resize(ef.header.funcCount);
    for (uint32_t i = 0; i < ef.header.funcCount; i++) {
        ef.symbolTable[i].name = readStr(f);
        f.read(reinterpret_cast<char*>(&ef.symbolTable[i].offset),     sizeof(uint32_t));
        f.read(reinterpret_cast<char*>(&ef.symbolTable[i].paramCount), sizeof(uint32_t));
    }

    ef.relocTable.resize(ef.header.relocCount);
    for (uint32_t i = 0; i < ef.header.relocCount; i++) {
        f.read(reinterpret_cast<char*>(&ef.relocTable[i].instrIndex), sizeof(uint32_t));
        f.read(reinterpret_cast<char*>(&ef.relocTable[i].funcIndex),  sizeof(uint32_t));
    }

    ef.jumpTable.resize(ef.header.funcCount);
    for (uint32_t i = 0; i < ef.header.funcCount; i++)
        ef.jumpTable[i] = ef.symbolTable[i].offset;

    for (auto& re : ef.relocTable) {
        uint32_t absOffset = ef.jumpTable[re.funcIndex];
        ef.code[re.instrIndex].leftIdx = absOffset;
    }

    return ef;
}
