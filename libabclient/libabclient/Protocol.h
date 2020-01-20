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
