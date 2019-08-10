#pragma once

#include <limits>
#include <iterator>
#include <cstring>
#include <vector>

namespace IO {

class PropReadStream
{
private:
    const char* p_ = nullptr;
    const char* end_ = nullptr;
public:
    PropReadStream() = default;
    // non-copyable
    PropReadStream(const PropReadStream&) = delete;
    PropReadStream& operator=(const PropReadStream&) = delete;
    ~PropReadStream() = default;

    void Init(const char* p, size_t size)
    {
        p_ = p;
        end_ = p + size;
    }
    size_t GetSize() const { return static_cast<size_t>(end_ - p_); }
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
#endif
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
    ~PropWriteStream() = default;

    const char* GetStream(size_t& size) const
    {
        size = buffer_.size();
        return buffer_.data();
    }
    size_t GetSize() const
    {
        return buffer_.size();
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
            // Error, too long
            Write<uint16_t>(0);
            return;
        }
        // Write length of string
        Write(static_cast<uint16_t>(len));
        // Copy the string itself
        std::copy(add.begin(), add.end(), std::back_inserter(buffer_));
    }
};

}
