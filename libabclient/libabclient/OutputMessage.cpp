#include "stdafx.h"
#include "OutputMessage.h"
#include <exception>
#include "Utils.h"
#include <abcrypto.hpp>
#include <AB/ProtocolCodes.h>
#include <lz4.h>

namespace Client {

std::vector<std::shared_ptr<OutputMessage>> OutputMessage::pool_;

std::shared_ptr<OutputMessage> OutputMessage::New()
{
    if (pool_.size() == 0)
    {
        for (size_t i = 0; i < MESSAGEPOOL_COUNT; ++i)
            pool_.push_back(std::make_shared<OutputMessage>());
    }
    std::shared_ptr<OutputMessage> msg = pool_.back();
    pool_.pop_back();
    return msg;
}

OutputMessage::OutputMessage() :
    info_({})
{ }

void OutputMessage::AddPaddingBytes(int bytes, uint8_t byte)
{
    if (bytes <= 0)
        return;
    CheckWrite(bytes);
    memset((void*)&buffer_[info_.pos], byte, static_cast<size_t>(bytes));
    info_.pos += static_cast<uint16_t>(bytes);
    info_.size += static_cast<uint16_t>(bytes);
}

void OutputMessage::AddString(const std::string& value)
{
    size_t len = value.length();

    if (len > OUTPUTMESSAGE_MAX_STRING_LEN)
        throw std::runtime_error("String too long");

    CheckWrite(static_cast<int>(len) + 2);
    Add<uint16_t>(static_cast<uint16_t>(len));
#ifdef _MSC_VER
    memcpy_s(reinterpret_cast<char*>(buffer_ + info_.pos), OUTPUTMESSAGE_BUFFER_SIZE - len, value.c_str(), len);
#else
    memcpy(reinterpret_cast<char*>(buffer_ + info_.pos), value.c_str(), len);
#endif
    info_.pos += static_cast<uint16_t>(len);
    info_.size += static_cast<uint16_t>(len);
}

void OutputMessage::AddStringEncrypted(const std::string& value)
{
    if (value.empty())
    {
        AddString(value);
        return;
    }

    size_t len = value.length();
    if (len > OUTPUTMESSAGE_MAX_STRING_LEN)
        throw std::runtime_error("String too long");

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
    xxtea_enc(buffer, static_cast<uint32_t>(len) / 4, AB::ENC_KEY);
    std::string encString(buff, len);
    AddString(encString);
    delete[] buff;
}

bool OutputMessage::CanWrite(int bytes)
{
    if (static_cast<size_t>(info_.pos) + static_cast<size_t>(bytes) <= OUTPUTMESSAGE_BUFFER_SIZE)
        return true;
    return false;
}

void OutputMessage::CheckWrite(int bytes)
{
    if (!CanWrite(bytes))
        throw std::runtime_error("OutputMessage max buffer size reached");
}

void OutputMessage::WriteChecksum()
{
    uint32_t checksum = AdlerChecksum(buffer_ + info_.headerPos, info_.size);
    assert(info_.headerPos - 4 >= 0);
    info_.headerPos -= 4;
    Set<uint32_t>(info_.headerPos, checksum);
    info_.size += 4;
}

void OutputMessage::WriteMessageSize()
{
    assert(info_.headerPos - 2 >= 0);
    info_.headerPos -= 2;
    Set<uint16_t>(info_.headerPos, info_.size);
    info_.size += 2;
}

bool OutputMessage::Compress()
{
    char buff[OUTPUTMESSAGE_BUFFER_SIZE];
    const char* src = reinterpret_cast<const char*>(buffer_ + OUTPUTMESSAGE_HEADER_SIZE);
    int size = LZ4_compress_default(src, buff, info_.size, OUTPUTMESSAGE_HEADER_SIZE);
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
