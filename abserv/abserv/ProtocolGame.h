#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"

class ProtocolGame : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Game Protocol"; };
public:
    ProtocolGame(std::shared_ptr<Connection> connection);
    ~ProtocolGame();
};

