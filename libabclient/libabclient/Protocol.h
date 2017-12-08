#pragma once

#include <string>
#include <stdint.h>
#include "Connection.h"
#include <memory>
#pragma warning(push)
#pragma warning(disable: 4592)
#include <asio.hpp>
#pragma warning(pop)
#include "Connection.h"
#include "InputMessage.h"
#include "OutputMessage.h"
#include <stdint.h>
#include "Defines.h"
#include <abcrypto.hpp>

namespace Client {

class Protocol : public std::enable_shared_from_this<Protocol>
{
public:
    typedef std::function<void(const std::error_code&)> ErrorCallback;
    typedef std::function<void(uint8_t)> ProtocolErrorCallback;
private:
    std::shared_ptr<InputMessage> inputMessage_;
    void InternalRecvHeader(uint8_t* buffer, uint16_t size);
    void InternalRecvData(uint8_t* buffer, uint16_t size);
protected:
    bool checksumEnabled_;
    DH_KEY sharedKey_;
    std::shared_ptr<Connection> connection_;
    ErrorCallback errorCallback_;
    ProtocolErrorCallback protocolErrorCallback_;
    virtual void OnError(const asio::error_code& err);
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
    Protocol();
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

    virtual void Send(const std::shared_ptr<OutputMessage>& message);
    virtual void Receive();
    void SetErrorCallback(const ErrorCallback& errorCallback) { errorCallback_ = errorCallback; }
    void SetProtocolErrorCallback(const ProtocolErrorCallback& errorCallback) { protocolErrorCallback_ = errorCallback; }
};

}
