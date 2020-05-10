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

namespace sa {
namespace bits {

template <typename T, typename U>
void set(T& bit_set, U bits)
{
    bit_set |= static_cast<T>(bits);
}

template <typename T, typename U>
void un_set(T& bit_set, U bits)
{
    bit_set &= ~static_cast<T>(bits);
}

template <typename T, typename U>
[[nodiscard]] bool is_set(T bit_set, U bit)
{
    return (bit_set & static_cast<T>(bit)) == static_cast<T>(bit) && (bit != 0 || bit_set == static_cast<T>(bit));
}

template <typename T, typename U>
[[nodiscard]] bool is_any_set(T bit_set, U bits)
{
    return (bit_set & static_cast<T>(bits)) != 0 && (bits != 0 || bit_set == static_cast<T>(bits));
}

template <typename T, typename U>
[[nodiscard]] bool equals(T bit_set, U bits)
{
    return bit_set == static_cast<T>(bits);
}

}
}
