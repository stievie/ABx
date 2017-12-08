#pragma once

#include <memory>
#include "Connection.h"
#include "Logger.h"
#include <abcrypto.hpp>

namespace Net {

class OutputMessage;

class Protocol : public std::enable_shared_from_this<Protocol>
{
private:
protected:
    /// DH shared key
    DH_KEY dhKey_;
    const std::weak_ptr<Connection> connection_;
    std::shared_ptr<OutputMessage> outputBuffer_;
    bool checksumEnabled_;
    void AESEncrypt(OutputMessage& message);
    bool AESDecrypt(NetworkMessage& message);
    /// Sets the shared key
    void SetDHKey(const DH_KEY* key)
    {
        memcpy_s(dhKey_, DH_KEY_LENGTH, key, DH_KEY_LENGTH);
    }
    const DH_KEY& GetDHKey() const
    {
        return dhKey_;
    }
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
        checksumEnabled_(false)
    {
    }
#if defined(_DEBUG)
    virtual ~Protocol()
    {
#ifdef DEBUG_NET
//        LOG_DEBUG << std::endl;
#endif
    }
#else
    virtual ~Protocol() = default;
#endif
    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;

    virtual void OnSendMessage(const std::shared_ptr<OutputMessage>& message) const;
    void OnRecvMessage(NetworkMessage& message);

    virtual void OnRecvFirstMessage(NetworkMessage& msg) = 0;
    virtual void OnConnect() {}

    virtual void ParsePacket(NetworkMessage& message) {
        AB_UNUSED(message);
    }

    bool IsConnectionExpired() const { return connection_.expired(); }
    std::shared_ptr<Connection> GetConnection() const { return connection_.lock(); }

    std::shared_ptr<OutputMessage> GetOutputBuffer(int32_t size);
    void ResetOutputBuffer();
    uint32_t GetIP()
    {
        if (auto c = GetConnection())
            return c->GetIP();
        return 0;
    }
    std::shared_ptr<OutputMessage>& GetCurrentBuffer()
    {
        return outputBuffer_;
    }

    void Send(std::shared_ptr<OutputMessage> message)
    {
        if (auto conn = GetConnection())
        {
            conn->Send(message);
        }
    }
};

}
