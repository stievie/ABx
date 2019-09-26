#pragma once

#include "Connection.h"
#include "Logger.h"
#include <abcrypto.hpp>
#include <sa/SharedPtr.h>
#include "OutputMessage.h"

namespace Net {

class Protocol : public std::enable_shared_from_this<Protocol>
{
protected:
    std::weak_ptr<Connection> connection_;
    sa::SharedPtr<OutputMessage> outputBuffer_;
    bool checksumEnabled_;
    bool compressionEnabled_;
    bool encryptionEnabled_;
    DH_KEY encKey_;
    void XTEAEncrypt(OutputMessage& msg) const;
    bool XTEADecrypt(NetworkMessage& msg) const;

    void Disconnect() const
    {
        if (auto conn = GetConnection())
            conn->Close();
    }
    virtual void Release() {}

    friend class Connection;
public:
    explicit Protocol(std::shared_ptr<Connection> connection) :
        connection_(connection),
        checksumEnabled_(false),
        compressionEnabled_(false),
        encryptionEnabled_(false)
    {
    }
    virtual ~Protocol() = default;

    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;

    void SetEncKey(const uint32_t* key)
    {
        memcpy(&encKey_, key, sizeof(encKey_));
    }

    virtual void OnSendMessage(OutputMessage& message) const;
    void OnRecvMessage(NetworkMessage& message);

    virtual void OnRecvFirstMessage(NetworkMessage& msg) = 0;
    virtual void OnConnect() {}

    virtual void ParsePacket(NetworkMessage&) {}

    bool IsConnectionExpired() const { return connection_.expired(); }
    std::shared_ptr<Connection> GetConnection() const { return connection_.lock(); }

    sa::SharedPtr<OutputMessage> GetOutputBuffer(int32_t size);
    void ResetOutputBuffer();
    uint32_t GetIP()
    {
        if (auto c = GetConnection())
            return c->GetIP();
        return 0;
    }
    sa::SharedPtr<OutputMessage>& GetCurrentBuffer()
    {
        return outputBuffer_;
    }

    void Send(sa::SharedPtr<OutputMessage> message)
    {
        if (auto conn = GetConnection())
        {
            conn->Send(message);
        }
    }
};

}
