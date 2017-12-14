#pragma once

#include <sys/timeb.h>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);
std::string ConvertIPToString(uint32_t ip);
uint32_t ConvertStringToIP(const std::string& ip);
std::vector<std::string> Split(const std::string& str, const std::string& delim);
std::string Trim(const std::string& str, const std::string& whitespace = " \t");
bool FileExists(const std::string& name);
bool CopyFile(const std::string& src, const std::string& dst);
std::string NormalizeFilename(const std::string& filename);
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

inline int64_t AbTick()
{
    timeb t;
    ftime(&t);
    return int64_t(t.millitm) + int64_t(t.time) * 1000;
}

}
