#include "REPL.h"

#include <sstream>
#include <readline/readline.h>
#include <readline/history.h>

#include "TerminalColor/TerminalColor.h"

Line::Line(const std::string &s) : ss(s), empty(s.empty()) {}

Line::Line(const Line &line) : ss(line.ss.str()), empty(line.empty) {}

std::string Line::nextToken() {
    std::string s;
    ss >> s;
    return s;
}

std::string REPL::prompt;
std::vector<std::string> REPL::tabCompletionList;

void REPL::setPrompt(const std::string &prompt) {
    std::stringstream ss;
    ss << TerminalColor::ForegroundGreen << "(" << prompt << ")" << TerminalColor::Reset;
    REPL::prompt = ss.str() + " ";
}

char **REPL::tabCompletionCallback(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, REPL::tabCompletionGenerator);
}

char *REPL::tabCompletionGenerator(const char *text, int state) {
    static size_t i, len;

    if (!state) {
        i = 0;
        len = strlen(text);
    }

    while (i < tabCompletionList.size()) {
        const char *curr = tabCompletionList[i++].data();
        if (strncmp(curr, text, len) == 0) {
            return strdup(curr);
        }
    }

    return NULL;
}


void REPL::setTabCompletionList(const std::vector<std::string> &list) {
    tabCompletionList = list;
}

Line REPL::readline(bool &eof) {
    rl_attempted_completion_function = REPL::tabCompletionCallback;

    char *p = ::readline(REPL::prompt.c_str());
    if (!p) {
        eof = true;
        return Line("");
    }

    eof = false;

    std::string s(p);
    if (s.length() > 0) add_history(p);

    free(p);
    return Line(s);
}
