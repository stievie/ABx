#include "stdafx.h"
#include "NetworkMessage.h"
#include <abcrypto.hpp>
#include <AB/ProtocolCodes.h>
#include <lz4.h>
#include "Subsystems.h"
#include "Dispatcher.h"

namespace Net {

NetworkMessage::MessagePool NetworkMessage::sNetworkMessagePool;

void NetworkMessage::Delete(NetworkMessage* p)
{
    sNetworkMessagePool.deallocate(p, 1);
}

std::unique_ptr<NetworkMessage> NetworkMessage::GetNew()
{
    auto* ptr = sNetworkMessagePool.allocate(1, nullptr);
    assert(ptr);
    ptr->Reset();
    return std::unique_ptr<NetworkMessage>(ptr);
}

std::string NetworkMessage::GetString(uint16_t len /* = 0 */)
{
    if (len == 0)
    {
        len = Get<uint16_t>();
        if (len >= (NETWORKMESSAGE_BUFFER_SIZE - info_.position))
            return std::string();
    }
    if (!CanRead(len))
        return std::string();

    char* v = reinterpret_cast<char*>(buffer_) + info_.position;
    info_.position += len;
    return std::string(v, len);
}

std::string NetworkMessage::GetStringEncrypted()
{
    std::string encString = GetString();
    if (encString.empty())
        return encString;

    size_t len = encString.length();
    char* buff = new char[len + 1];
    memset(buff, 0, len + 1);
#ifdef _MSC_VER
    memcpy_s((char*)(buff), len, encString.data(), len);
#else
    memcpy((char*)(buff), encString.data(), len);
#endif
    uint32_t* buffer = (uint32_t*)(buff);
    xxtea_dec(buffer, (uint32_t)(len / 4), AB::ENC_KEY);
    std::string result(buff);
    delete[] buff;
    return result;
}

void NetworkMessage::AddPaddingBytes(uint32_t n)
{
    if (!CanAdd(n))
        return;

    memset((void*)&buffer_[info_.position], static_cast<int>(AB::GameProtocol::CodeLast), n);
    info_.length += static_cast<MsgSize_t>(n);
}

void NetworkMessage::AddBytes(const char* bytes, uint32_t size)
{
    if (!CanAdd(size) || size > 8192)
        return;

#ifdef _MSC_VER
    memcpy_s(buffer_ + info_.position, NETWORKMESSAGE_BUFFER_SIZE, bytes, size);
#else
    memcpy(buffer_ + info_.position, bytes, size);
#endif
    info_.position += static_cast<MsgSize_t>(size);
    info_.length += static_cast<MsgSize_t>(size);
}

void NetworkMessage::AddString(const std::string& value)
{
    uint16_t len = static_cast<uint16_t>(value.length());
    if (!CanAdd(len + 2) || len > 8192)
        return;
    Add<uint16_t>(len);
    // Allows also \0
#ifdef _MSC_VER
    memcpy_s(buffer_ + info_.position, NETWORKMESSAGE_BUFFER_SIZE, value.data(), len);
#else
    memcpy(buffer_ + info_.position, value.data(), len);
#endif
    info_.position += static_cast<MsgSize_t>(len);
    info_.length += static_cast<MsgSize_t>(len);
}

void NetworkMessage::AddString(const char* value)
{
    uint16_t len = static_cast<uint16_t>(strlen(value));
    if (!CanAdd(len + 2) || len > 8192)
        return;

    Add<uint16_t>(len);
#ifdef _MSC_VER
    memcpy_s(buffer_ + info_.position, NETWORKMESSAGE_BUFFER_SIZE, value, len);
#else
    memcpy(buffer_ + info_.position, value, len);
#endif
    info_.position += static_cast<MsgSize_t>(len);
    info_.length += static_cast<MsgSize_t>(len);
}

void NetworkMessage::AddStringEncrypted(const std::string& value)
{
    uint16_t len = static_cast<uint16_t>(value.length());
    if (!CanAdd(len + 2) || len > 8192)
        return;

    if (value.empty())
    {
        AddString(value);
        return;
    }

    //add bytes until reach 8 multiple
    if ((len % 8) != 0)
    {
        uint16_t n = 8 - (len % 8);
        len += n;
    }
    char* buff = new char[len];
    memset(buff, 0, len);
#ifdef _MSC_VER
    memcpy_s((char*)(buff), len, value.data(), len);
#else
    memcpy((char*)(buff), value.data(), len);
#endif
    uint32_t* buffer = (uint32_t*)(buff);
    xxtea_enc(buffer, len / 4, AB::ENC_KEY);
    std::string encString(buff, len);
    AddString(encString);
    delete[] buff;
}

bool NetworkMessage::Compress()
{
#if 0
    std::unique_ptr<char[]> buff = std::make_unique<char[]>(NETWORKMESSAGE_BUFFER_SIZE);
    const char* src = reinterpret_cast<const char*>(buffer_ + HeaderLength);
    int size = LZ4_compress_default(src, buff.get(), GetSize(), NETWORKMESSAGE_BUFFER_SIZE);
    if (size > 0)
    {
/*
#ifdef _MSC_VER
        memcpy_s(buffer_ + HeaderLength, NETWORKMESSAGE_BUFFER_SIZE, buff, size);
#else
        memcpy(buffer_ + HeaderLength, buff, size);
#endif
        info_.position = INITIAL_BUFFER_POSITION;
        info_.length = size;
        */
        return true;
    }
#endif
    return false;
}

bool NetworkMessage::Uncompress()
{
#if 0
    std::unique_ptr<char[]> buff = std::make_unique<char[]>(NETWORKMESSAGE_BUFFER_SIZE);
    const char* src = reinterpret_cast<const char*>(buffer_ + HeaderLength);
    int size = LZ4_decompress_safe(src, buff.get(), GetSize(), NETWORKMESSAGE_BUFFER_SIZE);
    if (size > 0)
    {
        /*
#ifdef _MSC_VER
        memcpy_s(buffer_ + HeaderLength, NETWORKMESSAGE_BUFFER_SIZE, buff, size);
#else
        memcpy(buffer_ + HeaderLength, buff, size);
#endif
        info_.position = INITIAL_BUFFER_POSITION;
        info_.length = size;
        */
        return true;
    }
#endif
    return true;
}

}
