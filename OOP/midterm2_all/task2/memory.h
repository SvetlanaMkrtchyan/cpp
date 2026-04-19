#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

struct MemRegion {
    uint64_t base;
    uint64_t size;
};

struct Memory {
    uint64_t   totalSize;
    MemRegion  codeRegion;
    MemRegion  dataRegion;
    MemRegion  stackRegion;
    MemRegion  heapRegion;
    uint64_t   sp;
    uint64_t   hp;
    std::vector<uint8_t> mem;

    explicit Memory(uint64_t total = 1 << 24) : totalSize(total) {
        uint64_t q = total / 4;
        codeRegion  = { 0,     q };
        dataRegion  = { q,     q };
        stackRegion = { q * 2, q };
        heapRegion  = { q * 3, q };
        sp = stackRegion.base + stackRegion.size;
        hp = heapRegion.base;
        mem.resize(total, 0);
    }

    void writeDouble(uint64_t addr, double val) {
        if (addr + 8 > totalSize) throw std::runtime_error("Memory write out of bounds");
        memcpy(&mem[addr], &val, 8);
    }

    double readDouble(uint64_t addr) const {
        if (addr + 8 > totalSize) throw std::runtime_error("Memory read out of bounds");
        double v;
        memcpy(&v, &mem[addr], 8);
        return v;
    }

    void write32(uint64_t addr, uint32_t val) {
        if (addr + 4 > totalSize) throw std::runtime_error("Memory write out of bounds");
        memcpy(&mem[addr], &val, 4);
    }

    uint32_t read32(uint64_t addr) const {
        if (addr + 4 > totalSize) throw std::runtime_error("Memory read out of bounds");
        uint32_t v;
        memcpy(&v, &mem[addr], 4);
        return v;
    }

    uint64_t stackPush(double val) {
        if (sp < stackRegion.base + 8) throw std::runtime_error("Stack overflow");
        sp -= 8;
        writeDouble(sp, val);
        return sp;
    }

    double stackPop() {
        if (sp >= stackRegion.base + stackRegion.size) throw std::runtime_error("Stack underflow");
        double val = readDouble(sp);
        sp += 8;
        return val;
    }

    uint64_t heapAlloc(uint64_t bytes) {
        if (hp + bytes > heapRegion.base + heapRegion.size) throw std::runtime_error("Heap overflow");
        uint64_t ptr = hp;
        hp += bytes;
        return ptr;
    }

    void dumpLayout() const {
        printf("High  0x%08llX  +------------------+\n", (unsigned long long)(totalSize));
        printf("                   |       Heap       |  HP=0x%08llX\n", (unsigned long long)hp);
        printf("      0x%08llX  +------------------+\n", (unsigned long long)heapRegion.base);
        printf("                   |      Stack       |  SP=0x%08llX\n", (unsigned long long)sp);
        printf("      0x%08llX  +------------------+\n", (unsigned long long)stackRegion.base);
        printf("                   |       Data       |\n");
        printf("      0x%08llX  +------------------+\n", (unsigned long long)dataRegion.base);
        printf("                   |       Code       |\n");
        printf("Low   0x00000000   +------------------+\n\n");
    }
};
