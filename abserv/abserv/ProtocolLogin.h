#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include <stdint.h>
#include <AB/ProtocolCodes.h>

namespace Net {

class ProtocolLogin final : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = AB::ProtocolLoginId };
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Login Protocol"; };
public:
    explicit ProtocolLogin(std::shared_ptr<Connection> connection) :
        Protocol(connection)
    {
        checksumEnabled_ = ProtocolLogin::UseChecksum;
    }

    void OnRecvFirstMessage(NetworkMessage& message) final;
private:
    void DisconnectClient(uint8_t error);
    void SendCharacterList(const std::string& accountName, const std::string& password);
    void SendKeyExchange();
};

}
