#pragma once

#include <string>
#include <vector>
#include <limits>
#include <iterator>
#include <stdint.h>

class PropReadStream
{
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

        memcpy_s(&r, sizeof(T), p_, sizeof(T));
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

        char* str = new char[len + 1];
        memcpy_s(str, len, p_, len);
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
private:
    std::vector<char> buffer_;
public:
    PropWriteStream() = default;
    // non-copyable
    PropWriteStream(const PropWriteStream&) = delete;
    PropWriteStream& operator=(const PropWriteStream&) = delete;

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
