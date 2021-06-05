#include "Util.h"

#include <algorithm>
#include <regex>
#include <sstream>

#include "Ensure/Ensure.h"

void formatTable(std::vector<std::vector<std::string>> &table, std::vector<size_t> columnMinWidth) {
    if (table.empty()) return;

    size_t columnCount = table[0].size();
    for (const auto &row : table)
        if (columnCount != row.size())
            ERROR("Column count mismatch");
    columnMinWidth.resize(columnCount);

    static std::regex regexAnsi("\x1b\\[([0-9,A-Z]{1,2}(;[0-9]{1,2})?(;[0-9]{3})?)?[m|K]?");
    std::vector<size_t> columnLength(columnCount, 15);
    std::vector<std::vector<size_t>> cellLength(table.size());
    for (size_t i = 0; i < table.size(); i++) {
        auto &row = table[i];
        cellLength[i].resize(row.size());

        for (size_t j = 0; j < row.size(); j++) {
            std::string removedAnsi = std::regex_replace(row[j], regexAnsi, "");
            cellLength[i][j] = removedAnsi.length();

            columnLength[j] = std::max(
                std::max(
                    columnLength[j],
                    removedAnsi.length() + 1
                ),
                columnMinWidth[j]
            );
        }
    }

    for (size_t i = 0; i < table.size(); i++)
        for (size_t j = 0; j < table[i].size(); j++)
            table[i][j] += std::string(columnLength[j] - cellLength[i][j], ' ');
}
