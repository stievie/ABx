#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include <stdint.h>

namespace Net {

class ProtocolStatus : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0xFF };
    enum { UseChecksum = false };
    static const char* ProtocolName() { return "Status Protocol"; };
public:
    ProtocolStatus(std::shared_ptr<Connection> connection);
    ~ProtocolStatus();

    void OnRecvFirstMessage(NetworkMessage& msg) override;
};

}
