#include "stdafx.h"
#include "OutputMessage.h"
#include <exception>
#include "Utils.h"
#include <abcrypto.hpp>
#include <AB/ProtocolCodes.h>
#include <lz4.h>

#include "DebugNew.h"

namespace Client {

std::vector<std::unique_ptr<OutputMessage>> OutputMessage::pool_;

std::shared_ptr<OutputMessage> OutputMessage::New()
{
    if (pool_.size() == 0)
    {
        for (size_t i = 0; i < MESSAGEPOOL_COUNT; ++i)
            pool_.push_back(std::make_unique<OutputMessage>());
    }
    auto msg = std::move(pool_.back());
    pool_.pop_back();
    return msg;
}

OutputMessage::OutputMessage()
{
    Reset();
}

void OutputMessage::AddPaddingBytes(int bytes, uint8_t byte)
{
    if (bytes <= 0)
        return;
    CheckWrite(bytes);
    memset((void*)&buffer_[pos_], byte, bytes);
    pos_ += static_cast<uint16_t>(bytes);
    size_ += static_cast<uint16_t>(bytes);
}

void OutputMessage::AddString(const std::string& value)
{
    size_t len = value.length();

    if (len > MaxStringLength)
        throw std::runtime_error("String too long");

    CheckWrite(static_cast<int>(len) + 2);
    Add<uint16_t>(static_cast<uint16_t>(len));
#ifdef _MSC_VER
    memcpy_s((char*)(buffer_ + pos_), MaxBufferSize - len, value.c_str(), len);
#else
    memcpy((char*)(buffer_ + pos_), value.c_str(), len);
#endif
    pos_ += static_cast<uint16_t>(len);
    size_ += static_cast<uint16_t>(len);
}

void OutputMessage::AddStringEncrypted(const std::string& value)
{
    size_t len = value.length();
    if (len > MaxStringLength)
        throw std::runtime_error("String too long");

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
    xxtea_enc(buffer, static_cast<uint32_t>(len) / 4, AB::ENC_KEY);
    std::string encString(buff, len);
    AddString(encString);
    delete[] buff;
}

bool OutputMessage::CanWrite(int bytes)
{
    if (pos_ + bytes > MaxBufferSize)
        return false;
    return true;
}

void OutputMessage::CheckWrite(int bytes)
{
    if (!CanWrite(bytes))
        throw std::runtime_error("OutputMessage max buffer size reached");
}

void OutputMessage::WriteChecksum()
{
    uint32_t checksum = AdlerChecksum(buffer_ + headerPos_, size_);
    assert(headerPos_ - 4 >= 0);
    headerPos_ -= 4;
    Set<uint32_t>(headerPos_, checksum);
    size_ += 4;
}

void OutputMessage::WriteMessageSize()
{
    assert(headerPos_ - 2 >= 0);
    headerPos_ -= 2;
    Set<uint16_t>(headerPos_, size_);
    size_ += 2;
}

bool OutputMessage::Compress()
{
    char buff[MaxBufferSize];
    const char* src = reinterpret_cast<const char*>(buffer_ + MaxHeaderSize);
    int size = LZ4_compress_default(src, buff, size_, MaxBufferSize);
    if (size > 0)
    {
/*
#ifdef _MSC_VER
        memcpy_s(buffer_ + MaxHeaderSize, MaxBufferSize, buff, size);
#else
        memcpy(buffer_ + MaxHeaderSize, buff, size);
#endif
        size_ = static_cast<uint16_t>(size);
        pos_ = MaxHeaderSize;
        headerPos_ = MaxHeaderSize;
        */
        return true;
    }
    return false;
}

}
