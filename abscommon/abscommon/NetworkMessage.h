#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <vector>
#include <sa/PoolAllocator.h>

namespace Net {

class NetworkMessage;

/// Size of the NetworkMessage class, the buffer size is then NETWORKMESSAGE_MAXSIZE - sizeof(NetworkMessageInfo)
constexpr size_t NETWORKMESSAGE_MAXSIZE = 2048;
/// Size of the pool in Byte. Need around 2 messages per player
constexpr size_t NETWORKMESSAGE_POOLSIZE = NETWORKMESSAGE_MAXSIZE * 1024;

class NetworkMessage
{
private:
    using MessagePool = sa::PoolAllocator<NetworkMessage, NETWORKMESSAGE_POOLSIZE, NETWORKMESSAGE_MAXSIZE>;
    static MessagePool sNetworkMessagePool;
public:
    static void Delete(NetworkMessage* p);
    /// Pre allocated network messages
    static std::unique_ptr<NetworkMessage> GetNew();
    static sa::PoolInfo GetPoolInfo();
public:
    NetworkMessage() = default;
public:
    using MsgSize_t = uint16_t;
protected:
    struct NetworkMessageInfo
    {
        MsgSize_t length = 0;
        MsgSize_t position = INITIAL_BUFFER_POSITION;
    };
public:
    // Headers:
    // 2 bytes for unencrypted message size
    // 4 bytes for checksum
    // 2 bytes for encrypted message size
    static constexpr MsgSize_t INITIAL_BUFFER_POSITION = 8;
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
        if ((info_.position + static_cast<uint16_t>(size)) > (info_.length + 8) ||
            static_cast<uint16_t>(size) >= (NETWORKMESSAGE_BUFFER_SIZE - info_.position))
        {
            return false;
        }
        return true;
    }
public:
    void Reset()
    {
        info_ = {};
    }

    int32_t GetMessageLength() const { return info_.length; }

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
    std::string GetString(uint16_t len = 0);
    std::string GetStringEncrypted();
    uint32_t PeekU32()
    {
        uint32_t v = *(uint32_t*)(buffer_ + info_.position);
        return v;
    }

    /// Add function
    void AddPaddingBytes(uint32_t n);
    void AddBytes(const char* bytes, uint32_t size);
    void AddByte(uint8_t value)
    {
        if (!CanAdd(1))
            return;
        buffer_[info_.position++] = value;
        info_.length++;
    }
    void AddString(const std::string& value);
    void AddString(const char* value);
    void AddStringEncrypted(const std::string& value);
    template <typename T>
    void Add(T value)
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

    int32_t GetHeaderSize()
    {
        return (int32_t)(buffer_[0] | buffer_[1] << 8);
    }
    /// Other function
    void Skip(int bytes)
    {
        info_.position += (MsgSize_t)bytes;
    }

    uint8_t* GetBuffer() { return buffer_; }
    const uint8_t* GetBuffer() const { return buffer_; }
    uint8_t* GetBodyBuffer()
    {
        info_.position = 2;
        return buffer_ + HeaderLength;
    }
    int32_t GetSize() const { return static_cast<int32_t>(info_.length); }
    void SetSize(int32_t size) { info_.length = static_cast<MsgSize_t>(size); }
    int32_t GetReadPos() const { return static_cast<int32_t>(info_.position); }

    bool Compress();
    bool Uncompress();
};

static_assert(NETWORKMESSAGE_MAXSIZE == sizeof(NetworkMessage));

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
