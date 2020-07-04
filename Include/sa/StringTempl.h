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
#include <sa/Assert.h>

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
bool PatternMatch(const std::basic_string<charType>& str,
    const std::basic_string<charType>& pattern)
{
    enum class State
    {
        Exact,
        Any,
        AnyRepeat
    };

    const charType* s = str.c_str();
    const charType* p = pattern.c_str();
    const charType* q = 0;
    State state = State::Exact;

    bool match = true;

    while (match && *p)
    {
        switch (*p)
        {
        case '*':
            state = State::AnyRepeat;
            q = p + 1;
            break;
        case '?':
            state = State::Any;
            break;
        default:
            state = State::Exact;
            break;
        }

        if (*s == 0)
            break;

        switch (state)
        {
        case State::Exact:
            match = ToLower<charType>(*s) == ToLower<charType>(*p);
            ++s;
            ++p;
            break;
        case State::Any:
            match = true;
            ++s;
            ++p;
            break;
        case State::AnyRepeat:
            match = true;
            ++s;
            if (ToLower<charType>(*s) == ToLower<charType>(*q))
                ++p;
            break;
        }
    }

    if (state == State::AnyRepeat)
        return ToLower<charType>(*s) == ToLower<charType>(*q);
    if (state == State::Any)
        return ToLower<charType>(*s) == ToLower<charType>(*p);
    return match && (ToLower<charType>(*s) == ToLower<charType>(*p));
}

}
