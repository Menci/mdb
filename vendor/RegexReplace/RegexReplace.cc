#include "RegexReplace.h"

// https://stackoverflow.com/questions/22617209/regex-replace-with-callback-in-c11

std::string regexReplace(const std::string &input, const std::regex &re, std::function<std::string (const std::smatch &)> f) {
    std::string s;

    using iterator = std::string::const_iterator;
    typename std::smatch::difference_type positionOfLastMatch = 0;
    auto endOfLastMatch = input.cbegin();

    auto callback = [&] (const std::smatch &match) {
        auto positionOfThisMatch = match.position(0);
        auto diff = positionOfThisMatch - positionOfLastMatch;

        auto startOfThisMatch = endOfLastMatch;
        std::advance(startOfThisMatch, diff);

        s.append(endOfLastMatch, startOfThisMatch);
        s.append(f(match));

        auto lengthOfMatch = match.length(0);

        positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

        endOfLastMatch = startOfThisMatch;
        std::advance(endOfLastMatch, lengthOfMatch);
    };

    std::regex_iterator<iterator> begin(input.cbegin(), input.cend(), re), end;
    std::for_each(begin, end, callback);

    s.append(endOfLastMatch, input.cend());

    return s;
}
