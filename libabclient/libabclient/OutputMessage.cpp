#include "stdafx.h"
#include "OutputMessage.h"
#include <exception>
#include "Utils.h"

namespace Client {

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
    uint16_t len = static_cast<uint16_t>(value.length());

    if (len > MaxStringLength)
        throw std::exception("String too long");

    CheckWrite(len + 2);
    Add<uint16_t>(len);
    memcpy_s((char*)(buffer_ + pos_), MaxBufferSize - len, value.c_str(), len);
    pos_ += len;
    size_ += len;
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
        throw std::exception("OutputMessage max buffer size reached");
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

}
