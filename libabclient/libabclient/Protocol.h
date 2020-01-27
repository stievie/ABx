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

#include "Connection.h"
#include "Connection.h"
#include "InputMessage.h"
#include "OutputMessage.h"
#include <abcrypto.hpp>
#include <AB/DHKeys.hpp>
#include <AB/ProtocolCodes.h>

namespace Client {

class Protocol : public std::enable_shared_from_this<Protocol>
{
public:
    typedef std::function<void(ConnectionError connectionError, const std::error_code&)> ErrorCallback;
    typedef std::function<void(AB::ErrorCodes)> ProtocolErrorCallback;
private:
    std::shared_ptr<InputMessage> inputMessage_;
    void InternalRecvHeader(uint8_t* buffer, uint16_t size);
    void InternalRecvData(uint8_t* buffer, uint16_t size);
    bool XTEADecrypt(InputMessage& inputMessage);
    void XTEAEncrypt(OutputMessage& outputMessage);
protected:
    asio::io_service& ioService_;
    std::shared_ptr<Connection> connection_;
    bool checksumEnabled_;
    bool compressionEnabled_;
    bool encryptEnabled_;
    // Our (client) key pair
    Crypto::DHKeys& keys_;
    /// Shared key
    DH_KEY encKey_;
    ErrorCallback errorCallback_;
    ProtocolErrorCallback protocolErrorCallback_;
    virtual void OnError(ConnectionError connectionError, const asio::error_code& err);
    virtual void OnConnect() {}
    virtual void OnReceive(InputMessage&) { }
    void ProtocolError(AB::ErrorCodes err)
    {
        if (protocolErrorCallback_)
            protocolErrorCallback_(err);
    }
public:
    Protocol(Crypto::DHKeys& keys, asio::io_service& ioService);
    Protocol(const Protocol&) = delete;
    virtual ~Protocol();

    // Connect to server. Calls OnConnect virtual function on success.
    void Connect(const std::string& host, uint16_t port);
    // Connect to server. Calls onConnect callback function on success. Does not call the OnConnect virtual function.
    void Connect(const std::string& host, uint16_t port, std::function<void()>&& onConnect);
    void Disconnect();

    bool IsConnected() const
    {
        return (connection_ && connection_->IsConnected());
    }
    bool IsConnecting() const
    {
        return (connection_ && connection_->IsConnecting());
    }
    uint32_t GetIp() const
    {
        if (IsConnected())
            return connection_->GetIp();
        return 0;
    }

    void SetEncKey(const uint32_t* key)
    {
        memcpy(&encKey_, key, sizeof(encKey_));
    }

    virtual void Send(OutputMessage& message);
    virtual void Receive();
    void SetErrorCallback(const ErrorCallback& errorCallback) { errorCallback_ = errorCallback; }
    void SetProtocolErrorCallback(const ProtocolErrorCallback& errorCallback) { protocolErrorCallback_ = errorCallback; }
};

}
