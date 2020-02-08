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

#include "stdafx.h"
#include "MessageBuffer.h"

namespace IPC {

bool MessageBuffer::DecodeHeader()
{
    bodyLength_ = static_cast<size_t>(data_[0] | data_[1] << 8);
    if (bodyLength_ > MaxBodyLength)
        return false;
    type_ = *reinterpret_cast<uint32_t*>(data_ + 2);
    return true;
}

void MessageBuffer::EncodeHeader()
{
    data_[0] = static_cast<uint8_t>(bodyLength_);
    data_[1] = static_cast<uint8_t>(bodyLength_ >> 8);

#ifdef _MSC_VER
    memcpy_s(data_ + 2, sizeof(size_t), &type_, sizeof(size_t));
#else
    memcpy(data_ + 2, &type_, sizeof(size_t));
#endif
}

void MessageBuffer::AddString(const std::string& value)
{
    uint16_t len = static_cast<uint16_t>(value.length());
    if (!CanWrite(len + 2) || len > 8192)
        return;
    Add<uint16_t>(len);
    // Allows also \0
#ifdef _MSC_VER
    memcpy_s(data_ + pos_, MaxBodyLength, value.data(), len);
#else
    memcpy(data_ + pos_, value.data(), len);
#endif
    pos_ += static_cast<size_t>(len);
    bodyLength_ += static_cast<size_t>(len);
}

std::string MessageBuffer::GetString()
{
    size_t len = Get<uint16_t>();
    if (len >= (MaxBodyLength - pos_))
        return std::string();

    if (!CanRead(len))
        return std::string();

    char* v = reinterpret_cast<char*>(data_) + pos_;
    pos_ += len;
    return std::string(v, len);
}

}