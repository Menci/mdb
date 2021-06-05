#ifndef _MENCI_MDB_UTIL_H
#define _MENCI_MDB_UTIL_H

#include <vector>
#include <string>
#include <cstdint>

void formatTable(std::vector<std::vector<std::string>> &table, std::vector<size_t> columnMinWidth = {});

#endif // _MENCI_MDB_UTIL_H
