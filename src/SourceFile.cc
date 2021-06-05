#include "SourceFile.h"

#include <cstdio>

#include "Ensure/Ensure.h"

SourceFile::SourceFile(const std::string &filename) {
    FILE *f;
    ENSURE_NONZERO_ERRNO(f = fopen(filename.c_str(), "r"));

    while (true) {
        char *p = nullptr;
        size_t n = 0;
        ssize_t length;
        ENSURE_ERRNO(length = getline(&p, &n, f));

        if (length == -1) break;

        p[length - 1] = '\0';
        lines.push_back(p);

        free(p);
    }
}

bool SourceFile::hasLine(size_t i) const {
    return i > 0 && i - 1 < lines.size();
}

std::string SourceFile::getLine(size_t i) const {
    return i > 0 && i - 1 < lines.size() ? lines[i - 1] : "";
}
