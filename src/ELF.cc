#include "ELF.h"

#include <sstream>
#include <set>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <spawn.h>

#include "Ensure/Ensure.h"

Symbol::Symbol() : strtab(nullptr), sym(nullptr) {}

Symbol::Symbol(const char *strtab, const Elf64_Sym *sym) : strtab(strtab), sym(sym) {}

const char *Symbol::getName() {
    return &strtab[sym->st_name];
}

uintptr_t Symbol::getAddress() {
    return sym->st_value;
}

uintptr_t Symbol::getEndAddress() {
    return sym->st_value + sym->st_size;
}

Symbol::operator bool() {
    return sym;
}

ELF::ELF(const std::string &filename) {
    ENSURE_ERRNO(fd = open(filename.c_str(), O_RDONLY));
    ENSURE_ERRNO(fstat(fd, &st));
    ENSURE_ERRNO(p = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0));

    ehdr = (Elf64_Ehdr *)p;

    if (p[0] != 0x7f || strncmp((char *)(&p[1]), "ELF", 3))
        ERROR("Not an ELF file");
    if (ehdr->e_type != ET_EXEC)
        ERROR("Not an ELF executable file, maybe a dynamic linked file?");
    if (ehdr->e_shstrndx == 0 || ehdr->e_shoff == 0 || ehdr->e_shnum == 0)
        printf("ELF section header table not found");

    phdr = (Elf64_Phdr *)(p + ehdr->e_phoff);
    shdr = (Elf64_Shdr *)(p + ehdr->e_shoff);

    addr2line = std::unique_ptr<ChildProcess>(new ChildProcess("addr2line", {(std::string)"-e", filename}));
}

ELF::ELF(ELF &&other) {
    fd = other.fd;
    st = other.st;
    p = other.p;
    ehdr = other.ehdr;
    phdr = other.phdr;
    shdr = other.shdr;
    addr2line = std::move(other.addr2line);

    other.p = nullptr;
}

ELF::~ELF() {
    if (p) {
        ENSURE_ERRNO(munmap((void *)p, st.st_size));
        ENSURE_ERRNO(close(fd));
    }
}

void ELF::forEachSymbol(std::function<bool (const char *, const Elf64_Sym *)> callback) const {
    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            const char *strtab = (const char *)(&p[shdr[shdr[i].sh_link].sh_offset]);
            const Elf64_Sym *symtab = (const Elf64_Sym *)(&p[shdr[i].sh_offset]);
            for (size_t j = 0; j < shdr[i].sh_size / sizeof(Elf64_Sym); j++, symtab++)
                if (callback(strtab, symtab)) return;
        }
    }
}

std::vector<std::string> ELF::getAllSymbolNames() const {
    std::set<std::string> set;

    forEachSymbol([&] (const char *strtab, const Elf64_Sym *sym) {
        set.emplace(&strtab[sym->st_name]);
        return false;
    });

    return std::vector<std::string>{set.begin(), set.end()};
}

Symbol ELF::lookupSymbol(const std::string &name) const {
    Symbol result;
    forEachSymbol([&] (const char *strtab, const Elf64_Sym *sym) {
        if (name == &strtab[sym->st_name]) {
            result = Symbol(strtab, sym);
            return true;
        }

        return false;
    });

    return result;
}

Symbol ELF::lookupSymbol(uintptr_t address) const {
    Symbol result;
    forEachSymbol([&] (const char *strtab, const Elf64_Sym *sym) {
        if (address >= sym->st_value && address < sym->st_value + sym->st_size) {
            result = Symbol(strtab, sym);
            return true;
        }

        return false;
    });

    return result;
}

bool ELF::addressToLine(uintptr_t address, std::string &filename, size_t &line) {
    std::stringstream ss;
    ss << "0x" << std::hex << address;

    addr2line->writeLine(ss.str());
    std::string result = addr2line->readLine();
    if (result == "??:0" || result == "??:?") return false;

    size_t pos = result.find_last_of(':');
    ENSURE(pos != std::string::npos);

    filename = result.substr(0, pos);
    line = std::stoul(result.substr(pos + 1));
    return true;
}
