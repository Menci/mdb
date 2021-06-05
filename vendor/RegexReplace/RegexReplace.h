#ifndef _MENCI_REGEXREPLACE_H
#define _MENCI_REGEXREPLACE_H

#include <functional>
#include <string>
#include <regex>

std::string regexReplace(const std::string &input, const std::regex &re, std::function<std::string (const std::smatch &)> f);

#endif // _MENCI_REGEXREPLACE_H
