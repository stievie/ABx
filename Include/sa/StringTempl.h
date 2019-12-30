#pragma once

#include <algorithm>
#include <string>
#include <vector>

// Some string templates.

namespace sa {

template <typename charType>
std::basic_string<charType> Trim(const std::basic_string<charType>& str,
    const std::basic_string<charType>& whitespace = " \t")
{
    // Left
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return std::basic_string<charType>(); // no content

    // Right
    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

template <typename charType>
std::basic_string<charType> LeftTrim(const std::basic_string<charType>& str,
    const std::basic_string<charType>& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::basic_string<charType>::npos)
        return ""; // no content

    return str.substr(strBegin, std::basic_string<charType>::npos);
}

/// Replace all occurrences of search with replace in subject
/// Returns true if it replaced something
template <typename charType>
bool ReplaceSubstring(std::basic_string<charType>& subject,
    const std::basic_string<charType>& search,
    const std::basic_string<charType>& replace)
{
    if (search.empty())
        return false;
    typename std::basic_string<charType>::size_type pos = 0;
    bool result = false;
    while ((pos = subject.find(search, pos)) != std::basic_string<charType>::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
        result = true;
    }
    return result;
}

// Strip out all characters that are not valid for an identifier
template <typename charType>
void MakeIdent(std::basic_string<charType>& s)
{
    s = Trim<charType>(s, " \t\\/");
    using string_type = std::basic_string<charType>;
    static string_type invalidChars("\\/:?\"<>|.+-*");
    std::transform(s.begin(), s.end(), s.begin(), [](auto c) {
        return (invalidChars.find(c) == std::basic_string<charType>::npos) ? c : '_';
    });
    static string_type digits("1234567890");
    if (digits.find(s.front()) != string_type::npos)
        s.erase(s.begin());
}

template <typename charType>
std::basic_string<charType> CombineString(const std::vector<std::basic_string<charType>>& strings,
    const std::basic_string<charType>& delim)
{
    std::basic_string<charType> res;
    for (const auto& s : strings)
    {
        res += s + delim;
    }
    return Trim(res, delim);
}

}
