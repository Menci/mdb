#ifndef _MENCI_MDB_REPL_H
#define _MENCI_MDB_REPL_H

#include <sstream>
#include <string>
#include <vector>

class Line {
private:
    std::stringstream ss;

public:
    bool empty;

private:
    Line(const std::string &s);

    friend class REPL;

public:
    Line(const Line &line);

    std::string nextToken();
};

class REPL {
private:
    static std::string prompt;
    static std::vector<std::string> tabCompletionList;

    static char **tabCompletionCallback(const char *text, int start, int end);
    static char *tabCompletionGenerator(const char *text, int state);
    

public:
    static void setPrompt(const std::string &prompt);
    static Line readline(bool &eof);
    static void setTabCompletionList(const std::vector<std::string> &list);
};

#endif // _MENCI_MDB_REPL_H
