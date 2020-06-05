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
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <vector>
#include <sa/PoolAllocator.h>
#include <sa/Noncopyable.h>

namespace Net {

class NetworkMessage;

/// Size of the NetworkMessage class, the buffer size is then NETWORKMESSAGE_MAXSIZE - sizeof(NetworkMessageInfo)
/// OutputMessage should be exactly 4096 bytes, because then 100 friends and 100 Guild members fit in one
/// message.
static constexpr size_t NETWORKMESSAGE_MAXSIZE = 4096 - sizeof(uint32_t);

class NetworkMessage
{
    NON_COPYABLE(NetworkMessage)
public:
    using MessagePool = sa::PoolAllocator<NetworkMessage, NETWORKMESSAGE_MAXSIZE>;
    using MsgSize_t = uint16_t;
    // Headers:
    // 2 bytes for unencrypted message size
    // 4 bytes for checksum
    // 2 bytes for encrypted message size
    static constexpr MsgSize_t INITIAL_BUFFER_POSITION = 8;
    static void Delete(NetworkMessage* p);
    static std::unique_ptr<NetworkMessage> GetNew();
    static sa::PoolInfo GetPoolInfo();
    static unsigned GetPoolUsage();
    NetworkMessage();
protected:
    struct NetworkMessageInfo
    {
        MsgSize_t length = 0;
        MsgSize_t position = INITIAL_BUFFER_POSITION;
    };
public:
    static constexpr size_t NETWORKMESSAGE_BUFFER_SIZE = NETWORKMESSAGE_MAXSIZE - sizeof(NetworkMessageInfo);
    enum { HeaderLength = 2 };
    enum { ChecksumLength = 4 };
    enum { XteaMultiple = 8 };
    enum { MaxBodyLength = NETWORKMESSAGE_BUFFER_SIZE - HeaderLength - ChecksumLength - XteaMultiple };
    enum { MaxProtocolBodyLength = MaxBodyLength - 10 };
protected:
    NetworkMessageInfo info_;

    uint8_t buffer_[NETWORKMESSAGE_BUFFER_SIZE];
    bool CanAdd(uint32_t size) const
    {
        return (size + info_.position < MaxBodyLength);
    }
    bool CanRead(size_t size)
    {
        if ((info_.position + static_cast<uint16_t>(size)) > (info_.length + INITIAL_BUFFER_POSITION) ||
            static_cast<uint16_t>(size) >= (NETWORKMESSAGE_BUFFER_SIZE - info_.position))
        {
            return false;
        }
        return true;
    }
    std::string GetStringEncrypted();
    void AddStringEncrypted(const std::string& value);
    void AddString(const std::string& value);
public:
    void Reset()
    {
        info_.length = 0;
        info_.position = INITIAL_BUFFER_POSITION;
    }

    /// Read functions
    uint8_t GetByte()
    {
        if (!CanRead(1))
            return 0;
        return buffer_[info_.position++];
    }
    template <typename T>
    T Get()
    {
        if (!CanRead(sizeof(T)))
            return 0;
        T v;
#ifdef _MSC_VER
        memcpy_s(&v, sizeof(T), buffer_ + info_.position, sizeof(T));
#else
        memcpy(&v, buffer_ + info_.position, sizeof(T));
#endif
        info_.position += sizeof(T);
        return v;
    }
    template <typename T>
    T GetDecrypted()
    {
        return Get<T>();
    }
    std::string GetString(uint16_t len = 0);
    uint32_t PeekU32()
    {
        uint32_t v = *reinterpret_cast<uint32_t*>(buffer_ + info_.position);
        return v;
    }

    /// Add function
    void AddPaddingBytes(uint32_t n);
    void AddBytes(const char* bytes, uint32_t size);
    template<typename T>
    void AddByte(T value)
    {
        static_assert(sizeof(T) == 1, "T is not a byte type");
        AddByte(static_cast<uint8_t>(value));
    }
    void AddByte(uint8_t value)
    {
        if (!CanAdd(1))
            return;
        buffer_[info_.position++] = value;
        info_.length++;
    }
    template <typename T>
    void Add(const T& value)
    {
        if (!CanAdd(sizeof(T)))
            return;

#ifdef _MSC_VER
        memcpy_s(buffer_ + info_.position, sizeof(T), &value, sizeof(T));
#else
        memcpy(buffer_ + info_.position, &value, sizeof(T));
#endif
        info_.position += sizeof(T);
        info_.length += sizeof(T);
    }
    template <typename T>
    void AddEncryted(const T& value)
    {
        Add<T>(value);
    }

    int32_t GetHeaderSize()
    {
        return static_cast<int32_t>(buffer_[0] | buffer_[1] << 8);
    }
    /// Other function
    void Skip(int bytes)
    {
        info_.position += static_cast<MsgSize_t>(bytes);
    }

    uint8_t* GetBuffer() { return buffer_; }
    const uint8_t* GetBuffer() const { return buffer_; }
    uint8_t* GetBodyBuffer()
    {
        return buffer_ + HeaderLength;
    }
    const uint8_t* GetBodyBuffer() const
    {
        return buffer_ + HeaderLength;
    }
    int32_t GetSize() const { return static_cast<int32_t>(info_.length); }
    void SetSize(int32_t size) { info_.length = static_cast<MsgSize_t>(size); }
    int32_t GetReadPos() const { return static_cast<int32_t>(info_.position); }
    void SetReadPos(int32_t value) { info_.position = static_cast<MsgSize_t>(value); }
    int32_t GetSpace() const { return MaxBodyLength - info_.length; }

    bool Compress();
    bool Uncompress();
};

template <>
inline void NetworkMessage::Add<std::string>(const std::string& value)
{
    AddString(value);
}
template<>
inline void NetworkMessage::AddEncryted<std::string>(const std::string& value)
{
    AddStringEncrypted(value);
}
template <>
inline void NetworkMessage::Add<bool>(const bool& value)
{
    Add<uint8_t>(value ? 1 : 0);
}
template <>
inline std::string NetworkMessage::Get<std::string>()
{
    return GetString();
}
template<>
inline std::string NetworkMessage::GetDecrypted<std::string>()
{
    return GetStringEncrypted();
}
template <>
inline bool NetworkMessage::Get<bool>()
{
    return Get<uint8_t>() != 0;
}

static_assert(NETWORKMESSAGE_MAXSIZE == sizeof(NetworkMessage), "NETWORKMESSAGE_MAXSIZE doesn't match sizeof(NetworkMessage)");

}

namespace std {

/// All unique_ptr<NetworkMessage> are deleted from the pool
template <>
struct default_delete<Net::NetworkMessage> {
    default_delete() = default;
    void operator()(Net::NetworkMessage* p) const noexcept { Net::NetworkMessage::Delete(p); }
};

/// This means make_unique<NetworkMessage> must also create them from the pool.
/// This also means make_unique<NetworkMessage>(new NetworkMessage()) is not allowed.
template <>
inline unique_ptr<Net::NetworkMessage> make_unique<Net::NetworkMessage>()
{
    return Net::NetworkMessage::GetNew();
}

template <>
inline shared_ptr<Net::NetworkMessage> make_shared<Net::NetworkMessage>()
{
    // This doesnt work with shared_ptr's! Can not customize the deleter of shared_ptr's.
    throw std::runtime_error("NetworkMessage can not be used with a shared_ptr");
}

}
