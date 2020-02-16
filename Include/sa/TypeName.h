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

namespace details {
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
constexpr int is_char(const char* str, const char n)
{
    return *str == n;
}
}

/// Compile time typeid(T).name()
/// constexpr auto res = sa::TypeName<Foo::Bar>::Get();
template<typename T>
struct TypeName
{
    /// Will return "Foo::Bar"
    static constexpr auto Get()
    {
#if defined(__GNUC__) || defined(__clang__)
        // GCC will set __PRETTY_FUNCTION__ to something like:
        // sa::TypeName<T>::Get() [with T = Foo::Bar]
        // Clang: sa::TypeName<Foo::Bar>::Get() [T = Foo::Bar]
        constexpr const char* name = __PRETTY_FUNCTION__;
        constexpr int begin = details::first_char_pos(name, '=') + 2;
        constexpr int end = details::first_char_pos(name, ']');
#elif defined(_MSC_VER)
        // MSVC:
        // sa::TypeName<class Foo::Bar>::Get()
        // sa::TypeName<class Foo::Bar<Baz> >::Get()
        constexpr const char* name = __FUNCTION__;
        constexpr int ab = details::first_char_pos(name, '<') + 1;
        constexpr int begin = details::first_char_pos(name + ab, ' ') + ab + 1;
        constexpr int len = details::str_length(name);
        constexpr int lab = len - details::last_char_pos(name, '>', len);
        constexpr int end = details::is_char(name + lab - 1, ' ') ? lab - 1 : lab;
#endif

        static_assert(end > begin);
        constexpr int length = end - begin;
        constexpr const char* ptr = &name[begin];
        return std::string_view(ptr, length);
    }
};

}
