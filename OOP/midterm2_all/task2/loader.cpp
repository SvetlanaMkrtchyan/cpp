#include "loader.h"
#include <fstream>
#include <stdexcept>

using namespace std;

void saveExec(const string& path,
              const vector<Instruction>& code,
              const vector<double>&      constPool,
              const map<string, int>&    varMap,
              int                        entryPoint)
{
    ofstream f(path, ios::binary);
    if (!f) throw runtime_error("Cannot write: " + path);

    vector<string> varNames(varMap.size());
    for (auto& [name, id] : varMap) varNames[id] = name;

    ExecHeader hdr;
    hdr.magic      = EXEC_MAGIC;
    hdr.version    = EXEC_VERSION;
    hdr.codeSize   = code.size();
    hdr.dataSize   = varNames.size();
    hdr.constSize  = constPool.size();
    hdr.varCount   = varNames.size();
    hdr.entryPoint = entryPoint;

    f.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    f.write(reinterpret_cast<const char*>(code.data()), code.size() * sizeof(Instruction));

    f.write(reinterpret_cast<const char*>(constPool.data()), constPool.size() * sizeof(double));

    for (auto& name : varNames) {
        uint32_t len = name.size();
        f.write(reinterpret_cast<const char*>(&len), sizeof(len));
        f.write(name.data(), len);
    }
}

ExecFile loadExec(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) throw runtime_error("Cannot open: " + path);

    ExecFile ef;
    f.read(reinterpret_cast<char*>(&ef.header), sizeof(ExecHeader));

    if (ef.header.magic != EXEC_MAGIC)
        throw runtime_error("Invalid exec file magic");
    if (ef.header.version != EXEC_VERSION)
        throw runtime_error("Unsupported exec file version");

    ef.code.resize(ef.header.codeSize);
    f.read(reinterpret_cast<char*>(ef.code.data()), ef.header.codeSize * sizeof(Instruction));

    ef.constPool.resize(ef.header.constSize);
    f.read(reinterpret_cast<char*>(ef.constPool.data()), ef.header.constSize * sizeof(double));

    ef.varNames.resize(ef.header.varCount);
    for (uint32_t i = 0; i < ef.header.varCount; i++) {
        uint32_t len;
        f.read(reinterpret_cast<char*>(&len), sizeof(len));
        ef.varNames[i].resize(len);
        f.read(ef.varNames[i].data(), len);
    }

    return ef;
}
