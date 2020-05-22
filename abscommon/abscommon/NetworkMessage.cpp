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


#include "NetworkMessage.h"
#include <abcrypto.hpp>
#include <AB/ProtocolCodes.h>
#include <lz4.h>
#include "Subsystems.h"
#include "Dispatcher.h"
#include "Subsystems.h"
#include "Logger.h"

namespace Net {

void NetworkMessage::Delete(NetworkMessage* p)
{
    auto* pool = GetSubsystem<NetworkMessage::MessagePool>();
    if (!pool)
    {
        LOG_ERROR << "No NetworkMessage::MessagePool" << std::endl;
        return;
    }
    pool->deallocate(p, 1);
}

std::unique_ptr<NetworkMessage> NetworkMessage::GetNew()
{
    auto* pool = GetSubsystem<NetworkMessage::MessagePool>();
    if (!pool)
    {
        LOG_ERROR << "No NetworkMessage::MessagePool" << std::endl;
        return std::unique_ptr<NetworkMessage>();
    }
    auto* ptr = pool->allocate(1, nullptr);
    assert(ptr);
    ptr->Reset();
    return std::unique_ptr<NetworkMessage>(ptr);
}

sa::PoolInfo NetworkMessage::GetPoolInfo()
{
    auto* pool = GetSubsystem<NetworkMessage::MessagePool>();
    if (!pool)
    {
        LOG_ERROR << "No NetworkMessage::MessagePool" << std::endl;
        return { };
    }
    return pool->GetInfo();
}

unsigned NetworkMessage::GetPoolUsage()
{
    auto* pool = GetSubsystem<NetworkMessage::MessagePool>();
    if (pool)
        return pool->GetUsage();
    return 0;
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
    memcpy_s(buff, len, encString.data(), len);
#else
    memcpy(buff, encString.data(), len);
#endif
    uint32_t* buffer = reinterpret_cast<uint32_t*>(buff);
    xxtea_dec(buffer, static_cast<uint32_t>(len / 4), AB::ENC_KEY);
    std::string result(buff);
    delete[] buff;
    return result;
}

void NetworkMessage::AddPaddingBytes(uint32_t n)
{
    if (!CanAdd(n))
        return;

    memset(reinterpret_cast<void*>(&buffer_[info_.position]), static_cast<int>(AB::GameProtocol::ServerPacketType::__Last), n);
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
    memcpy_s(buff, len, value.data(), len);
#else
    memcpy(buff, value.data(), len);
#endif
    uint32_t* buffer = reinterpret_cast<uint32_t*>(buff);
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
