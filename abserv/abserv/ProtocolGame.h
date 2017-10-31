#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"

namespace Net {

class ProtocolGame : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Game Protocol"; };
public:
    explicit ProtocolGame(std::shared_ptr<Connection> connection);
    virtual ~ProtocolGame();

    void OnRecvFirstMessage(NetworkMessage& msg) override;
    void OnConnect() override;
};

}
