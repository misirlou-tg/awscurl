#pragma once
#include <string>
#include <utility>

// I found out about how to parse into HeaderValue from the CLI11 README.md "custom_parse" example
// https://github.com/CLIUtils/CLI11#examples
// (have to derive from std::pair because lexical_cast() below MUST be in the same namespace
//  as the type you want to parse into)
class HeaderValue : public std::pair<std::string, std::string>
{
    // Pull in all constructors for std::pair<std::string, std::string>
    using std::pair<std::string, std::string>::pair;
};

bool lexical_cast(const std::string &input, HeaderValue &headerValue)
{
#ifdef VERBOSE_LOGGING
    std::cout << "Parse headerValue: " << input << std::endl;
#endif

    auto indexSeparator = input.find(": ");
    if(indexSeparator == std::string::npos || indexSeparator == 0)
    {
        return false;
    }
    headerValue.first = input.substr(0, indexSeparator);
    headerValue.second = input.substr(indexSeparator + 2);
    return true;
}
