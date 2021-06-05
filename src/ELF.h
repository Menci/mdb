#ifndef _MENCI_MDB_ELF_H
#define _MENCI_MDB_ELF_H

#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>

#include <unistd.h>
#include <sys/stat.h>
#include <elf.h>

#include "ChildProcess/ChildProcess.h"

class Symbol {
    const char *strtab;
    const Elf64_Sym *sym;

public:
    Symbol();
    Symbol(const char *strtab, const Elf64_Sym *sym);
    const char *getName();
    uintptr_t getAddress();
    uintptr_t getEndAddress();
    operator bool();
};

class ELF {
    int fd;
    struct stat st;
    char *p;

    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Shdr *shdr;

    std::unique_ptr<ChildProcess> addr2line;

    void forEachSymbol(std::function<bool (const char *, const Elf64_Sym *)> callback) const;

public:
    ELF(const std::string &filename);
    ELF(const ELF &) = delete;
    ELF(ELF &&);
    ~ELF();

    std::vector<std::string> getAllSymbolNames() const;
    Symbol lookupSymbol(const std::string &name) const;
    Symbol lookupSymbol(uintptr_t address) const;

    bool addressToLine(uintptr_t address, std::string &filename, size_t &line);
};

#endif // _MENCI_MDB_ELF_H
