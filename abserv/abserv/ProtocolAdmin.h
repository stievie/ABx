#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include <stdint.h>

class ProtocolAdmin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0xFE };
    enum { UseChecksum = false };
    static const char* ProtocolName() { return "Admin Protocol"; };
public:
    ProtocolAdmin(std::shared_ptr<Connection> connection);
    ~ProtocolAdmin();

    void OnRecvFirstMessage(NetworkMessage& msg) override;
};

