#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include <stdint.h>

namespace Net {

class ProtocolLogin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0x01 };
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Login Protocol"; };
public:
    ProtocolLogin(std::shared_ptr<Connection> connection);
    virtual ~ProtocolLogin();

    void OnRecvFirstMessage(NetworkMessage& message) override;
private:
    void DisconnectClient(uint8_t error, const char* message);
    bool ParseFirstPacket(NetworkMessage& message);
};

}
