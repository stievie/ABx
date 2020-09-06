/**
 * Copyright 2020 Stefan Ascher
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

#include <stddef.h>
#include <string_view>
#include <vector>
#include <utility>
#include <sstream>
#include <optional>
#include <algorithm>
#include <sa/StringTempl.h>
#include <sa/Compiler.h>
#if defined(SA_MSVC)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#elif defined(SA_PLATFORM_LINUX)
#include <sys/types.h>
#endif

// Parse HTTP Range header
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Range

namespace sa {
namespace http {

struct range
{
    size_t start{ 0 };
    size_t end{ 0 };
    ssize_t length{ 0 };
    bool operator > (const range& rhs) const { return start > rhs.start; }
};

using ranges = std::vector<range>;

namespace details {

inline bool parse_range(size_t size, std::string_view value, range& result)
{
    auto dash_sign = value.find('-');
    if (dash_sign == std::string_view::npos)
        return false;
    std::string_view from = value.substr(0, dash_sign);
    std::string_view to = value.substr(dash_sign + 1);
    if (from.length() == 0 && to.length() == 0)
        return false;
    auto from_value = sa::to_number<size_t>(from);
    auto to_value = sa::to_number<size_t>(to);
    if (!from_value.has_value() && !to_value.has_value())
        return false;

    if (from_value.has_value())
        result.start = from_value.value();
    if (to_value.has_value())
        result.end = to_value.value();

    if (!from_value.has_value())
    {
        result.start = size - to_value.value();
        result.end = size;
    }
    else if (!to_value.has_value())
        result.end = size;
    result.length = result.end - result.start;
    return result.length > 0;
}

inline bool ranges_overlap(const range& a, const range& b)
{
    return a.end > b.start;
}

inline void combine_ranges(ranges& input)
{
    // Combine overlapping ranges into one bigger range
    ranges result(input);
    std::sort(result.begin(), result.end(), [](const range& a, const range& b) -> bool
    {
        return b > a;
    });

    for (ssize_t i = result.size() - 1; i > 0; --i)
    {
        auto& b = result[i];
        auto& a = result[i - 1];
        if (ranges_overlap(a, b))
        {
            a.start = std::min(a.start, b.start);
            a.end = std::max(a.end, b.end);
            a.length = a.end - a.start;
            result.erase(result.begin() + i);
        }
    }
    input = result;
}

}

inline bool is_full_range(size_t size, const range& range)
{
    if (range.end == 0)
        return true;
    if (range.start == 0 && (size_t)range.length >= size)
        return true;
    return false;
}

inline size_t content_length(const ranges& ranges)
{
    size_t result = 0;
    for (const auto& r : ranges)
        result += r.length;
    return result;
}

// Parse Range header, return a sorted list of ranges that do not overlap.
// size is the file size in bytes.
inline bool parse_ranges(size_t size, std::string_view header, ranges& result)
{
    auto equal_sign = header.find('=');
    if (equal_sign == std::string_view::npos)
        return false;
    std::string_view units = header.substr(0, equal_sign);
    if (units != "bytes")
        // Only support bytes unit
        return false;
    std::string_view header_value = header.substr(equal_sign + 1);

    std::vector<std::string> values = sa::Split(std::string(header_value), ",", false, false);
    for (auto& value : values)
    {
        value = sa::Trim<char>(value);
        range range_value;
        if (details::parse_range(size, value, range_value))
            result.push_back(std::move(range_value));
    }
    if (!result.empty())
        details::combine_ranges(result);

    // If it does not contain any valid ranges, it is clearly not a valid ranges header.
    return !result.empty();
}

}
}
