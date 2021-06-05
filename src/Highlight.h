#ifndef _MENCI_MDB_HIGHLIGHT_H
#define _MENCI_MDB_HIGHLIGHT_H

#include <string>

enum class HighlightLanguage {
    Cpp,
    ASM
};

class Highlight {
public:
    static std::string highlight(const std::string &string, HighlightLanguage language);
};

#endif // _MENCI_MDB_HIGHLIGHT_H
