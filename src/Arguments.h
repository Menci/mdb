#ifndef _MENCI_MDB_ARGUMENTS_H
#define _MENCI_MDB_ARGUMENTS_H

#include <string>

struct Arguments {
    std::string executable;
    size_t pid;
};

Arguments parseArguments(int argc, char *argv[]);

#endif // _MENCI_MDB_ARGUMENTS_H
