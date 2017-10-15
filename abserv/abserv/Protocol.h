#pragma once

#include <memory>
#include "Connection.h"
#include "RefCounted.h"

class OutputMessage;
class NetworkMessage;

class Protocol : public RefCounted
{
private:
    std::shared_ptr<Connection> connection_;
    uint32_t xteaKey_[4];
protected:
    void XTEAEncrypt(OutputMessage& message);
    void XTEADecrypt(NetworkMessage& message);
public:
    Protocol(std::shared_ptr<Connection> connection) :
        connection_(connection)
    {}
    Protocol(const Protocol&) = delete;
    ~Protocol() {}

    void OnSendMessage(std::shared_ptr<OutputMessage> message);
    void OnRecvMessage(NetworkMessage& message);

    virtual void OnRecvFirstMessage(NetworkMessage& msg) = 0;
    virtual void OnConnect() {}

    void SetConnection(std::shared_ptr<Connection> connection)
    {
        connection_ = connection;
    }
    std::shared_ptr<Connection> GetConnection() const { return connection_; }

    void Release();

};

