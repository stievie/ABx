#pragma once

#include "Protocol.h"
#include "Connection.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Character.h>
#include <AB/Packets/LoginPackets.h>

namespace Net {

class ProtocolLogin final : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = AB::ProtocolLoginId };
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Login Protocol"; }
public:
    explicit ProtocolLogin(std::shared_ptr<Connection> connection) :
        Protocol(connection)
    {
        checksumEnabled_ = ProtocolLogin::UseChecksum;
        SetEncKey(AB::ENC_KEY);
    }

    void OnRecvFirstMessage(NetworkMessage& message) override;
private:
    void DisconnectClient(AB::ErrorCodes error);

    void HandleLoginPacket(NetworkMessage& message);
    void HandleCreateAccountPacket(NetworkMessage& message);
    void HandleCreateCharacterPacket(NetworkMessage& message);
    void HandleDeleteCharacterPacket(NetworkMessage& message);
    void HandleAddAccountKeyPacket(NetworkMessage& message);
    void HandleGetOutpostsPacket(NetworkMessage& message);
    void HandleGetServersPacket(NetworkMessage& message);

    // These are passed by value because they are executed by the dispatcher later
    void AuthenticateSendCharacterList(AB::Packets::Client::Login::Login request);
    void SendOutposts(AB::Packets::Client::Login::GetOutposts request);
    void SendServers(AB::Packets::Client::Login::GetServers request);
    void CreateAccount(AB::Packets::Client::Login::CreateAccount request);
    void AddAccountKey(AB::Packets::Client::Login::AddAccountKey request);
    void CreatePlayer(AB::Packets::Client::Login::CreatePlayer request);
    void DeletePlayer(AB::Packets::Client::Login::DeleteCharacter request);
};

}
