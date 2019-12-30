#pragma once

#include "Logger.h"
#include <abcrypto.hpp>
#include <cstring>
#include "OutputMessage.h"

namespace Net {

class Connection;

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

    void Disconnect() const;
    virtual void Release() {}

    friend class Connection;
public:
    explicit Protocol(std::shared_ptr<Connection> connection) :
        connection_(connection),
        checksumEnabled_(false),
        compressionEnabled_(false),
        encryptionEnabled_(false)
    { }
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
    uint32_t GetIP();
    sa::SharedPtr<OutputMessage>& GetCurrentBuffer();

    void Send(sa::SharedPtr<OutputMessage>&& message);
};

}
