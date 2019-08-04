#pragma once

#include <algorithm>

namespace Utils {

/// Case insensitive string compare
bool StringEquals(const std::string& l, const std::string& r);
bool StringEquals(const std::wstring& l, const std::wstring& r);
/// On Windows case insensitive, otherwise case sensitive
bool SameFilename(const std::string& l, const std::string& r);
std::string ChangeFileExt(const std::string& fn, const std::string& ext);
std::string GetFileExt(const std::string& fn);
std::string ExtractFileDir(const std::string& fn);

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
std::vector<std::string> Split(const std::string& str, const std::string& delim);
bool IsNumber(const std::string& s);
std::string ConvertSize(size_t size);
uint32_t ConvertStringToIP(const std::string& ip);
std::string ConvertIPToString(uint32_t ip, bool mask = false);
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

/// Strip out all characters that are not valid for an identifier
template <typename charType>
void MakeIdent(std::basic_string<charType>& s)
{
    s = Trim<charType>(s, " \t\\/");
    static std::basic_string<charType> invalidChars("\\/:?\"<>|.+-*");
    std::transform(s.begin(), s.end(), s.begin(), [](auto c) {
        return (invalidChars.find(c) == std::string::npos) ? c : '_';
    });
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

std::string UrlEncode(const std::string& str);
std::string UrlDecode(const std::string& str);

}
