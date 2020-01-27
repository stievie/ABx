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

#include <type_traits>

namespace sa {

// To be used with fundamental types
// using int_type1 = sa::StrongType<int, struct IntTag1>;
// using int_type2 = sa::StrongType<int, struct IntTag2>;

template <typename T, typename Tag>
struct StrongType
{
    static_assert(std::is_fundamental<T>::value, "T must be a fundamental type");
    StrongType() = default;
    StrongType(T value) :
        value(value)
    { }
    operator T () const { return value; }
    T value;
};

// To be used with structs and classes
template <typename T, typename Tag>
struct StrongClass : public T
{
    static_assert(std::is_class<T>::value, "T must be a class or struct type");
};

}
