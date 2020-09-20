/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <cwctype>
#include <regex>
#include <optional>
#include <sa/Assert.h>

// Some string templates.

namespace sa {

template<typename T>
inline std::optional<T> to_number(std::string_view number)
{
    T result;
    std::string snumber(number);
    std::istringstream iss(snumber);
    iss >> result;
    if (iss.fail())
        return {};
    return result;
}

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

template <typename charType>
std::basic_string<charType> RightTrim(const std::basic_string<charType>& str,
    const std::basic_string<charType>& whitespace = " \t")
{
    const auto strEnd = str.find_last_not_of(whitespace);
    if (strEnd == std::basic_string<charType>::npos)
        return ""; // no content

    return str.substr(0, strEnd + 1);
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
    bool result = false;

    using string_type = std::basic_string<charType>;
    string_type newString;
    newString.reserve(subject.length());

    size_t lastPos = 0;
    size_t findPos;

    while ((findPos = subject.find(search, lastPos)) != string_type::npos)
    {
        newString.append(subject, lastPos, findPos - lastPos);
        newString += replace;
        lastPos = findPos + search.length();
        result = true;
    }

    newString += subject.substr(lastPos);

    subject.swap(newString);
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
    static const string_type digits("1234567890");
    if (digits.find(s.front()) != string_type::npos)
        s.erase(s.begin());
}

inline bool IsWhite(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isspace(*it)) ++it;
    return !s.empty() && it == s.end();
}

inline std::vector<std::string> Split(const std::string& str, const std::string& delim, bool keepEmpty = false, bool keepWhite = true)
{
    std::vector<std::string> result;

    size_t beg = 0;
    for (size_t end = 0; (end = str.find(delim, end)) != std::string::npos; ++end)
    {
        auto substring = str.substr(beg, end - beg);
        beg = end + delim.length();
        if (!keepEmpty && substring.empty())
            continue;
        if (!keepWhite && IsWhite(substring))
            continue;
        result.push_back(substring);
    }

    auto substring = str.substr(beg);
    if (substring.empty())
    {
        if (keepEmpty)
            result.push_back(substring);
    }
    else if (IsWhite(substring))
    {
        if (keepWhite)
            result.push_back(substring);
    }
    else
        result.push_back(substring);

    return result;
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

template <typename charType>
std::basic_string<charType> CombinePath(const std::basic_string<charType>& path,
    const std::basic_string<charType>& file)
{
    std::basic_string<charType> result = path;
    if (result.back() != '/' && result.back() != '\\' && file.front() != '/' && file.front() != '\\')
        result += "/";
    result += file;
    return result;
}

template <typename charType>
inline charType ToLower(charType c)
{
    ASSERT_FALSE();
}
template <>
inline char ToLower<char>(char c)
{
    return static_cast<char>(std::tolower(c));
}
template <>
inline wchar_t ToLower<wchar_t>(wchar_t c)
{
    return std::towlower(c);
}
template <typename charType>
inline charType ToUpper(charType c)
{
    ASSERT_FALSE();
}
template <>
inline char ToUpper<char>(char c)
{
    return static_cast<char>(std::toupper(c));
}
template <>
inline wchar_t ToUpper<wchar_t>(wchar_t c)
{
    return std::towupper(c);
}

template <typename charType>
inline std::basic_string<charType> StringToUpper(const std::basic_string<charType>& s)
{
    std::basic_string<charType> result;
    result.resize(s.length());
    for (size_t i = 0; i < s.length(); ++i)
        result[i] = ToUpper<charType>(s[i]);
    return result;
}
template <typename charType>
inline std::basic_string<charType> StringToLower(const std::basic_string<charType>& s)
{
    std::basic_string<charType> result;
    result.resize(s.length());
    for (size_t i = 0; i < s.length(); ++i)
        result[i] = ToLower<charType>(s[i]);
    return result;
}

template <typename charType>
bool PatternMatch(const std::basic_string<charType>& string,
    const std::basic_string<charType>& pattern)
{
    // http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html#evolution
    bool star = false;
    const charType* s;
    const charType* p;
    const charType* str = string.c_str();
    const charType* pat = pattern.c_str();

loopStart:
    for (s = str, p = pat; *s; ++s, ++p)
    {
        switch (*p) {
        case '?':
            if (*s == '.')
                goto starCheck;
            break;
        case '*':
            star = true;
            str = s, pat = p;
            do
            {
                ++pat;
            } while (*pat == '*');
            if (!*pat)
                return true;
            goto loopStart;
        default:
            if (ToLower<charType>(*s) != ToLower<charType>(*p))
                goto starCheck;
            break;
        }
    }
    while (*p == '*')
        ++p;
    return (!*p);
starCheck:
    if (!star)
        return false;
    str++;
    goto loopStart;
}

}
