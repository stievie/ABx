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
        if (*p == '*')
        {
            state = State::AnyRepeat;
            q = p + 1;
        }
        else if (*p == '?')
            state = State::Any;
        else
            state = State::Exact;

        if (*s == 0)
            break;

        switch (state)
        {
        case State::Exact:
            match = *s == *p;
            s++;
            p++;;
            break;
        case State::Any:
            match = true;
            s++;
            p++;
            break;
        case State::AnyRepeat:
            match = true;
            s++;
            if (*s == *q)
                p++;
            break;
        }
    }

    if (state == State::AnyRepeat)
        return *s == *q;
    if (state == State::Any)
        return *s == *p;
    return match && (*s == *p);
}

}
