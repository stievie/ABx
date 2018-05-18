#pragma once

namespace Utils {

std::string Trim(const std::string& str, const std::string& whitespace = " \t");
std::string LeftTrim(const std::string& str, const std::string& whitespace = " \t");
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

}
