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

#include <stdint.h>
#include <limits>
#include <vector>
#include <string>
#include <optional>

#define ABAI_STRINGIFY_INTERNAL(x) #x
#define ABAI_STRINGIFY(x) ABAI_STRINGIFY_INTERNAL(x)

namespace AI {

typedef uint32_t Id;
static constexpr Id INVALID_ID = std::numeric_limits<Id>::max();
using ArgumentsType = std::vector<std::string>;

template <typename T, std::enable_if_t<std::is_integral<T>::value || std::is_enum<T>::value, int> = 0>
inline bool GetArgument(const ArgumentsType& arguments, size_t index, T& result)
{
    if (index < arguments.size())
    {
        result = static_cast<T>(atoi(arguments[0].c_str()));
        return true;
    }
    return false;
}

template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
inline bool GetArgument(const ArgumentsType& arguments, size_t index, T& result)
{
    if (index < arguments.size())
    {
        result = static_cast<T>(atof(arguments[0].c_str()));
        return true;
    }
    return false;
}

template <typename T, std::enable_if_t<!std::is_integral<T>::value && !std::is_enum<T>::value && !std::is_floating_point<T>::value, int> = 0>
inline bool GetArgument(const ArgumentsType& arguments, size_t index, T& result)
{
    return false;
}

template<>
inline bool GetArgument<std::string>(const ArgumentsType& arguments, size_t index, std::string& result)
{
    if (index < arguments.size())
    {
        result = arguments.at(index);
        return true;
    }
    return false;
}

template<>
inline bool GetArgument<bool>(const ArgumentsType& arguments, size_t index, bool& result)
{
    if (index < arguments.size())
    {
        result = (arguments.at(index).compare("true") == 0 ||
            atoi(arguments[0].c_str()) != 0);
        return true;
    }
    return false;
}

}
