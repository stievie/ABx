#pragma once

#include <memory>
#include "Connection.h"

class OutputMessage;
class NetworkMessage;

class Protocol
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
    void OnRecvMessage(NetworkMessage* message);
    std::shared_ptr<Connection> GetConnection() const { return connection_; }

};

