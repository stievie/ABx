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

#include <string_view>

namespace sa {

constexpr int first_char_pos(const char* str, const char n)
{
    return (*str && *str != n) ? 1 + first_char_pos(str + 1, n) : 0;
}

constexpr int str_length(const char* str)
{
    return *str ? 1 + str_length(str + 1) : 0;
}

constexpr int last_char_pos(const char* str, const char n, int len)
{
    return (len && *(str + len) != n) ? 1 + last_char_pos(str, n, len - 1) : 0;
}

/// Compile time typeid(T).name()
/// constexpr auto res = sa::TypeName<Foo::Bar>::Get();
template<typename T>
struct TypeName
{
    /// GCC will return "Foo::Bar", MSVC "class Foo::Bar"
    static constexpr auto Get()
    {
#if defined(__GNUC__)
        // GCC will set __PRETTY_FUNCTION__ to something like:
        // sa::TypeName<T>::Get() [with T = Foo::Bar]
        constexpr const char* name = __PRETTY_FUNCTION__;
        constexpr int begin = first_char_pos(name, '=') + 2;
        constexpr int end = first_char_pos(name, ']');
#elif defined(_MSC_VER)
        // MSVC:
        // sa::TypeName<class Foo::Bar>::Get()
        constexpr const char* name = __FUNCTION__;
        constexpr int begin = first_char_pos(name, '<') + 1;
        constexpr int len = str_length(name);
        constexpr int end = len - last_char_pos(name, '>', len);
#endif

        static_assert(end > begin);
        constexpr int length = end - begin;
        constexpr const char* ptr = &name[begin];
        return std::string_view(ptr, length);
    }
};

}
