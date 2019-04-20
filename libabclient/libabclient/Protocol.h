#pragma once

#include "Connection.h"
#include "Connection.h"
#include "InputMessage.h"
#include "OutputMessage.h"
#include <abcrypto.hpp>
#include <AB/DHKeys.hpp>

namespace Client {

class Protocol : public std::enable_shared_from_this<Protocol>
{
public:
    typedef std::function<void(ConnectionError connectionError, const std::error_code&)> ErrorCallback;
    typedef std::function<void(uint8_t)> ProtocolErrorCallback;
private:
    std::shared_ptr<InputMessage> inputMessage_;
    void InternalRecvHeader(uint8_t* buffer, uint16_t size);
    void InternalRecvData(uint8_t* buffer, uint16_t size);
    bool XTEADecrypt(const std::shared_ptr<InputMessage>& inputMessage);
    void XTEAEncrypt(const std::shared_ptr<OutputMessage>& outputMessage);
protected:
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
    virtual void OnReceive(const std::shared_ptr<InputMessage>& message) {
        AB_UNUSED(message);
    }
    void ProtocolError(uint8_t err)
    {
        if (protocolErrorCallback_)
            protocolErrorCallback_(err);
    }
public:
    Protocol(Crypto::DHKeys& keys);
    Protocol(const Protocol&) = delete;
    ~Protocol();

    void Connect(const std::string& host, uint16_t port);
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

    virtual void Send(const std::shared_ptr<OutputMessage>& message);
    virtual void Receive();
    void SetErrorCallback(const ErrorCallback& errorCallback) { errorCallback_ = errorCallback; }
    void SetProtocolErrorCallback(const ProtocolErrorCallback& errorCallback) { protocolErrorCallback_ = errorCallback; }
};

}
