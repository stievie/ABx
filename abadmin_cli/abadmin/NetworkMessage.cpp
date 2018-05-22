#include "stdafx.h"
#include "NetworkMessage.h"
#include "Rsa.h"
#include <algorithm>
#include <iostream>

bool NetworkMessage::encryptionEnabled = false;
bool NetworkMessage::keySet = false;
uint32_t NetworkMessage::key[4] = { 0 };

NetworkMessage::NetworkMessage()
{
    Reset();
}

SocketCode NetworkMessage::ReadFromSocket(SOCKET socket, int timeout)
{
    int readBytes = 0;
    int sleepTime = 0;
    size_ = 0;

    do
    {
        // just read the size to avoid reading 2 messages at once
        int b = recv(socket, (char*)buffer_ + readBytes, 2 - readBytes, 0);
        if (b <= 0)
        {
            int errNum = ::WSAGetLastError();
            if (errNum == WSAEWOULDBLOCK)
            {
                b = 0;
                AB_SLEEP(10);
                sleepTime += 10;
                if (sleepTime > timeout)
                {
                    Reset();
                    return SocketCodeError;
                }
            }
            else
            {
                Reset();
                return SocketCodeError;
            }
        }
        readBytes += b;
    } while (readBytes < 2);

    size_ = readBytes;

    // for now we expect 2 bytes at once, it should not be splitted
    int datasize = buffer_[0] | buffer_[1] << 8;
    if ((size_ != 2) || (datasize > NETWORKMESSAGE_MAXSIZE - 2))
    {
        Reset();
        return SocketCodeError;
    }

    readBytes = 0;

    do
    {
        // read the real data
        int b = recv(socket, (char*)buffer_ + readBytes + 2, datasize - readBytes, 0);

        if (b <= 0)
        {
            int errnum;
#if defined WIN32 || defined __WINDOWS__
            errnum = ::WSAGetLastError();
#else
            errnum = errno;
#endif
            if (errnum == EWOULDBLOCK)
            {
                b = 0;
                AB_SLEEP(100);
                sleepTime += 100;
                if (sleepTime > timeout)
                {
                    Reset();
                    return SocketCodeTimeout;
                }
            }
            else
            {
                Reset();
                return SocketCodeError;
            }
        }

        readBytes += b;
        size_ += b;
    } while (readBytes < datasize);

    // we got something unexpected/incomplete
    if ((size_ <= 2) || ((buffer_[0] | buffer_[1] << 8) != size_ - 2))
    {
        Reset();
        return SocketCodeError;
    }

    readPos_ = 0;
    // Decrypt
    if (encryptionEnabled)
    {
        if (!keySet)
        {
            return SocketCodeError;
        }

        if ((size_ - 2) % 8 != 0)
        {
            return SocketCodeError;
        }

        XTEADecrypt();
        int tmp = Get<uint16_t>();
        if (tmp > size_ - 4)
        {
            return SocketCodeError;
        }
        size_ = tmp;
    }

    return SocketCodeOK;
}

SocketCode NetworkMessage::WriteToSocket(SOCKET socket, int timeout)
{
    if (size_ == 0)
        return SocketCodeOK;

    buffer_[2] = (unsigned char)(size_);
    buffer_[3] = (unsigned char)(size_ >> 8);

    int sendBytes = 0;
    int flags = 0;
    int sleepTime = 0;

    int start;
    if (encryptionEnabled)
    {
        if (!keySet)
        {
            // Key not set
            return SocketCodeError;
        }
        start = 0;
        XTEAEncrypt();
    }
    else
        start = 2;

    do
    {
        int b = send(socket, (char*)buffer_ + sendBytes + start, std::min(size_ - sendBytes + 2, 1000), flags);
        if (b <= 0) {
            int errnum;
#if defined WIN32 || defined __WINDOWS__
            errnum = ::WSAGetLastError();
#else
            errnum = errno;
#endif
            if (errnum == EWOULDBLOCK)
            {
                b = 0;
                AB_SLEEP(100);
                sleepTime += 100;
                if (sleepTime > timeout)
                    return SocketCodeTimeout;
            }
            else
                return SocketCodeError;
        }
        sendBytes += b;
    } while (sendBytes < size_ + 2);
    return SocketCodeOK;
}

void NetworkMessage::SetEncryptionState(bool state)
{
    encryptionEnabled = state;
}

void NetworkMessage::SetEncryptionKey(const uint32_t* _key)
{
    memcpy(key, _key, 16);
    keySet = true;
}

std::string NetworkMessage::GetString()
{
    uint16_t stringlen = Get<uint16_t>();
    if (stringlen >= (16384 - readPos_))
        return std::string();

    char* v = (char*)(buffer_ + readPos_);
    readPos_ += stringlen;
    return std::string(v, stringlen);
}

void NetworkMessage::AddString(const std::string& value)
{
    AddString(value.c_str());
}

void NetworkMessage::AddString(const char* value)
{
    uint32_t stringlen = (uint32_t)strlen(value);
    if (!CanAdd(stringlen + 2) || stringlen > 8192)
        return;

    Add<uint16_t>(stringlen);
    strcpy_s((char*)(buffer_ + readPos_), stringlen, value);
    readPos_ += stringlen;
    size_ += stringlen;
}

void NetworkMessage::Reset()
{
    size_ = 0;
    readPos_ = 4;
}

void NetworkMessage::XTEAEncrypt()
{
    uint32_t k[4];
    k[0] = key[0]; k[1] = key[1]; k[2] = key[2]; k[3] = key[3];

    //add bytes until reach 8 multiple
    uint32_t n;
    if (((size_ + 2) % 8) != 0)
    {
        n = 8 - ((size_ + 2) % 8);
        memset((void*)&buffer_[readPos_], 0, n);
        size_ += n;
    }

    unsigned long read_pos = 0;
    uint32_t* buffer = (uint32_t*)&buffer_[2];
    while (read_pos < (unsigned)size_ / 4)
    {
        uint32_t v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
        uint32_t delta = 0x61C88647;
        uint32_t sum = 0;

        for (long i = 0; i < 32; i++)
        {
            v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
            sum -= delta;
            v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
        }
        buffer[read_pos] = v0; buffer[read_pos + 1] = v1;
        read_pos += 2;
    }
    size_ += 2;

    *(uint16_t*)(buffer_) = size_;
}

void NetworkMessage::XTEADecrypt()
{
    uint32_t k[4];
    k[0] = key[0]; k[1] = key[1]; k[2] = key[2]; k[3] = key[3];

    uint32_t* buffer = (uint32_t*)&buffer_[2];
    unsigned long read_pos = 0;
    while (read_pos < (unsigned)size_ / 4)
    {
        uint32_t v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
        uint32_t delta = 0x61C88647;
        uint32_t sum = 0xC6EF3720;

        for (long i = 0; i < 32; i++)
        {
            v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
            sum += delta;
            v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
        }
        buffer[read_pos] = v0; buffer[read_pos + 1] = v1;
        read_pos += 2;
    }
}

bool NetworkMessage::RSAEncrypt()
{
    int newPos = readPos_ - 128;
    if (newPos < 0)
    {
        std::cout << "No valid packet size" << std::endl;
        return false;
    }
    if (!Rsa::Instance.Encrypt((char*)(buffer_ + newPos), 128))
        return false;
    return true;
}

