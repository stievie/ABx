#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include <stdint.h>

namespace Net {

class ProtocolLogin final : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0x01 };
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Login Protocol"; };
public:
    explicit ProtocolLogin(std::shared_ptr<Connection> connection) :
        Protocol(connection)
    {}

    void OnRecvFirstMessage(NetworkMessage& message) final;
private:
    void DisconnectClient(uint8_t error, const char* message);
    void GetCharacterList(const std::string& accountName, const std::string& password);
};

}
