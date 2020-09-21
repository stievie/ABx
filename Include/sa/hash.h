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

#include <string_view>
#include <charconv>
#include <iomanip>
#include <stdint.h>
#include <cstring>

namespace sa {

template<typename T, size_t Count>
class hash
{
    static_assert(Count != 0);
private:
    T data_[Count]{};
public:
    hash() {}
    // Construct from Hex string
    explicit hash(const std::string_view value)
    {
        constexpr size_t charsPerNum = sizeof(T) * 2;
        if (value.size() == charsPerNum * Count)
        {
            for (size_t i = 0; i < Count; ++i)
            {
                std::from_chars(
                    value.data() + (i * charsPerNum),
                    value.data() + (i * charsPerNum) + charsPerNum,
                    data_[i],
                    16
                );
            }
        }
    }
    hash(const hash& rhs)
    {
        std::copy(&rhs.data_[0], &rhs.data_[0] + Count * sizeof(T), &data_[0]);
    }
    hash& operator=(const hash& rhs)
    {
        if (&rhs == this)
            return *this;
        std::copy(&rhs.data_[0], &rhs.data_[0] + Count * sizeof(T), &data_[0]);
        return *this;
    }
    bool operator ==(const hash& rhs) const
    {
        return memcmp(&data_[0], &rhs.data_[0], Count * sizeof(T)) == 0;
    }
    bool operator !=(const hash& rhs) const
    {
        return !(*this == rhs);
    }
    friend std::ostream& operator << (std::ostream& os, const hash& value)
    {
        const uint8_t* buff = reinterpret_cast<const uint8_t*>(value.data());
        constexpr size_t c = sizeof(value.data_);
        for (size_t i = 0; i < c; ++i)
        {
            os << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned int)buff[i]);
        }
        return os;
    }
    bool empty() const
    {
        static T empty_data[Count]{};
        return memcmp(&data_[0], &empty_data[0], Count * sizeof(T)) == 0;
    }
    operator bool() const
    {
        return !empty();
    }

    const T* data() const { return &data_[0]; }
    T* data() { return &data_[0]; }
    constexpr size_t size() const { return Count * sizeof(T); }
};

}
