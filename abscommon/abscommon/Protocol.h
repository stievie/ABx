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

#include "Logger.h"
#include <abcrypto.hpp>
#include <cstring>
#include <sa/SmartPtr.h>
#include <sa/Noncopyable.h>

namespace Net {

class Connection;
class OutputMessage;
class NetworkMessage;

class Protocol : public std::enable_shared_from_this<Protocol>
{
    NON_COPYABLE(Protocol)
protected:
    std::weak_ptr<Connection> connection_;
    sa::SharedPtr<OutputMessage> outputBuffer_;
    bool encryptionEnabled_;
    DH_KEY encKey_;
    void XTEAEncrypt(OutputMessage& msg) const;
    bool XTEADecrypt(NetworkMessage& msg) const;

    void Disconnect() const;
    virtual void Release() {}

    friend class Connection;
public:
    explicit Protocol(std::shared_ptr<Connection> connection);
    virtual ~Protocol();

    void SetEncKey(const uint32_t* key)
    {
        memcpy(&encKey_, key, sizeof(encKey_));
    }

    virtual bool OnSendMessage(OutputMessage& message) const;
    void OnRecvMessage(NetworkMessage& message);

    virtual void OnRecvFirstMessage(NetworkMessage& msg) = 0;
    virtual void OnConnect() {}

    virtual void ParsePacket(NetworkMessage&) {}

    bool IsConnectionExpired() const { return connection_.expired(); }
    std::shared_ptr<Connection> GetConnection() const { return connection_.lock(); }

    sa::SharedPtr<OutputMessage> GetOutputBuffer(size_t size);
    void ResetOutputBuffer();
    uint32_t GetIP();
    sa::SharedPtr<OutputMessage> TakeCurrentBuffer();

    void Send(sa::SharedPtr<OutputMessage>&& message);
};

}
