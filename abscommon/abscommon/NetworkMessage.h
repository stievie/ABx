#pragma once

/// Size of the buffer (16kB)
#define NETWORKMESSAGE_MAXSIZE 16384

namespace Net {

/// Size of the pool in Byte (4MB)
static constexpr size_t NETWORKMESSAGE_POOLSIZE = 4096 * 1000;
/// Number of messages to preallocate
static constexpr size_t NETWORKMESSAGE_POOLCOUNT = NETWORKMESSAGE_POOLSIZE / NETWORKMESSAGE_MAXSIZE;

class NetworkMessage
{
private:
    static std::vector<std::unique_ptr<NetworkMessage>> pool_;
public:
    /// Pre allocated network messages
    static std::unique_ptr<NetworkMessage> GetNew();
public:
    using MsgSize_t = uint16_t;
    // Headers:
    // 2 bytes for unencrypted message size
    // 4 bytes for checksum
    // 2 bytes for encrypted message size
    static constexpr MsgSize_t INITIAL_BUFFER_POSITION = 8;
    enum { HeaderLength = 2 };
    enum { ChecksumLength = 4 };
    enum { XteaMultiple = 8 };
    enum { MaxBodyLength = NETWORKMESSAGE_MAXSIZE - HeaderLength - ChecksumLength - XteaMultiple };
    enum { MaxProtocolBodyLength = MaxBodyLength - 10 };
protected:
    struct NetworkMessageInfo
    {
        MsgSize_t length = 0;
        MsgSize_t position = INITIAL_BUFFER_POSITION;
    };

    NetworkMessageInfo info_;

    uint8_t buffer_[NETWORKMESSAGE_MAXSIZE];
    bool CanAdd(uint32_t size) const
    {
        return (size + info_.position < MaxBodyLength);
    }
    bool CanRead(size_t size)
    {
        if ((info_.position + static_cast<uint16_t>(size)) > (info_.length + 8) ||
            static_cast<uint16_t>(size) >= (NETWORKMESSAGE_MAXSIZE - info_.position))
        {
            return false;
        }
        return true;
    }
public:
    NetworkMessage() = default;
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
    int32_t GetSize() const { return info_.length; }
    void SetSize(int32_t size) { info_.length = (MsgSize_t)size; }
    int32_t GetReadPos() const { return info_.position; }

    bool Compress();
    bool Uncompress();
};

}
