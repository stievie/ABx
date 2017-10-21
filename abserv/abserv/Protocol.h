#pragma once

#include <memory>
#include "Connection.h"
#include "RefCounted.h"

namespace Net {

class OutputMessage;

class Protocol : public Utils::RefCounted
{
private:
    std::shared_ptr<Connection> connection_;
    uint32_t xteaKey_[4];
    void DeleteProtocolTask();
protected:
    std::shared_ptr<OutputMessage> outputBuffer_;
    bool encryptionEnabled_;
    bool rawMessages_;
    bool checksumEnabled_;
    void XTEAEncrypt(OutputMessage& message);
    bool XTEADecrypt(NetworkMessage& message);
public:
    Protocol(std::shared_ptr<Connection> connection) :
        connection_(connection),
        encryptionEnabled_(false),
        rawMessages_(false),
        checksumEnabled_(false)
    {
        memset(&xteaKey_, 0, sizeof(xteaKey_));
    }
    Protocol(const Protocol&) = delete;
    virtual ~Protocol() {}

    void OnSendMessage(std::shared_ptr<OutputMessage> message);
    void OnRecvMessage(NetworkMessage& message);

    virtual void OnRecvFirstMessage(NetworkMessage& msg) = 0;
    virtual void OnConnect() {}

    virtual void ParsePacket(NetworkMessage& message) {}

    void SetConnection(std::shared_ptr<Connection> connection)
    {
        connection_ = connection;
    }
    std::shared_ptr<Connection> GetConnection() const { return connection_; }
    std::shared_ptr<OutputMessage> GetOutputBuffer();
    uint32_t GetIP()
    {
        if (auto c = GetConnection())
            return c->GetIP();
        return 0;
    }

    void Send(std::shared_ptr<OutputMessage> message);
    void Disconnect();
    void Release();
};

}
