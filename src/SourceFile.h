#ifndef _MENCI_MDB_SOURCEFILE_H
#define _MENCI_MDB_SOURCEFILE_H

#include <fstream>
#include <string>
#include <vector>

class SourceFile {
    std::vector<std::string> lines;

public:
    SourceFile(const std::string &filename);
    bool hasLine(size_t i) const;
    std::string getLine(size_t i) const;
};

#endif // _MENCI_MDB_SOURCEFILE_H
