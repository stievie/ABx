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

#include <string>
#include <vector>
#include <limits>
#include <iterator>
#include <stdint.h>
#include <cstring>
#include <sa/Noncopyable.h>

class PropReadStream
{
    NON_COPYABLE(PropReadStream)
private:
    const char* p_ = nullptr;
    const char* start_ = nullptr;
    const char* end_ = nullptr;
public:
    PropReadStream() = default;
    PropReadStream(const char* p, size_t size) :
        p_(p),
        start_(p),
        end_(p + size)
    {}

    void Init(const char* p, size_t size)
    {
        p_ = p;
        start_ = p;
        end_ = p + size;
    }
    size_t GetSize() const { return end_ - p_; }
    const char* Buffer() const
    {
        return start_;
    }
    template <typename T>
    bool Read(T& r)
    {
        if (GetSize() < sizeof(T))
            return false;

#ifdef _MSC_VER
        memcpy_s(&r, sizeof(T), p_, sizeof(T));
#else
        memcpy(&r, p_, sizeof(T));
#endif
        p_ += sizeof(T);
        return true;
    }
    bool ReadString(std::string& r)
    {
        uint16_t len;
        if (!Read<uint16_t>(len))
            return false;

        if (GetSize() < len)
            return false;

        char* str = new char[static_cast<size_t>(len) + 1];
#ifdef _MSC_VER
        memcpy_s(str, len, p_, len);
#else
        memcpy(str, p_, len);
#endif // _MSC_VER
        str[len] = 0;
        r.assign(str, len);
        p_ += len;
        delete[] str;
        return true;
    }
    bool Skip(size_t n)
    {
        if (GetSize() < n)
            return false;
        p_ += n;
        return true;
    }
};

class PropWriteStream
{
    NON_COPYABLE(PropWriteStream)
private:
    std::vector<char> buffer_;
public:
    PropWriteStream() = default;

    const char* GetStream(size_t& size) const
    {
        size = buffer_.size();
        return buffer_.data();
    }
    void Clear()
    {
        buffer_.clear();
    }
    template <typename T>
    void Write(T add)
    {
        char* addr = reinterpret_cast<char*>(&add);
        std::copy(addr, addr + sizeof(T), std::back_inserter(buffer_));
    }
    void WriteString(const std::string& add)
    {
        size_t len = add.length();
        if (len > std::numeric_limits<uint16_t>::max())
        {
            Write<uint16_t>(0);
            return;
        }
        Write(static_cast<uint16_t>(len));
        std::copy(add.begin(), add.end(), std::back_inserter(buffer_));
    }
};
