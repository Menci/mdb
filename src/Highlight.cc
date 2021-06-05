#include "Highlight.h"

#include <sstream>

#include <srchilite/sourcehighlight.h>

#include "Ensure/Ensure.h"

srchilite::SourceHighlight sourceHighlight("esc.outlang");

std::string Highlight::highlight(const std::string &string, HighlightLanguage language) {
    const char *lang;
    switch (language) {
    case HighlightLanguage::Cpp:
        lang = "cpp.lang";
        break;
    case HighlightLanguage::ASM:
        lang = "asm.lang";
        break;
    default:
        ERROR("Invalid language");
    }
    std::stringstream in(string), out;
    sourceHighlight.highlight(in, out, lang);

    return out.str();
}
