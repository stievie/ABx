#pragma once

#include <string>
#include <stdint.h>
#include "Connection.h"
#include <memory>
#include <asio.hpp>
#include "Connection.h"
#include "InputMessage.h"
#include "OutputMessage.h"
#include <stdint.h>

class Protocol : public std::enable_shared_from_this<Protocol>
{
private:
    std::shared_ptr<InputMessage> inputMessage_;
    bool checksumEnabled_;
    bool xteaEnabled_;
    void InternalRecvHeader(uint8_t* buffer, uint16_t size);
    void InternalRecvData(uint8_t* buffer, uint16_t size);
    bool XteaDecrypt(const std::shared_ptr<InputMessage>& message);
    void XteaEncrypt(const std::shared_ptr<OutputMessage>& message);
protected:
    std::shared_ptr<Connection> connection_;
    uint32_t xteaKey_[4];
    virtual void OnError(const asio::error_code& err);
    virtual void OnConnect() {}
    virtual void OnReceive(const std::shared_ptr<InputMessage>& message) {}
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

    void GenerateXteaKey();
    void SetXteaKey(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
    virtual void Send(const std::shared_ptr<OutputMessage>& message);
    virtual void Receive();
};

