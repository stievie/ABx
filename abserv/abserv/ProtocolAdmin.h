#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include <stdint.h>

namespace Net {

enum ApMsg
{
    ApMsgLogin = 1
};

class ProtocolAdmin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0xFE };
    enum { UseChecksum = false };
    static const char* ProtocolName() { return "Admin Protocol"; };
private:
    enum ConnectionState
    {
        NotConnected,
        EncryptionToSet,
        EncryptionOK,
        NotloggedIn,
        LooggedIn
    };
    ConnectionState state_;
    uint32_t loginTries_;
    time_t lastCommand_;
    time_t startTime_;
public:
    ProtocolAdmin(std::shared_ptr<Connection> connection);
    virtual ~ProtocolAdmin() {}

    void OnRecvFirstMessage(NetworkMessage& msg) override;
    void ParsePacket(NetworkMessage& message) override;

    bool AllowIP(uint32_t clientIP);
};

}
