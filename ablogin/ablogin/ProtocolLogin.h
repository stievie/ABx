/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <AB/Entities/Character.h>
#include <AB/Packets/LoginPackets.h>
#include <AB/ProtocolCodes.h>
#include <abscommon/Connection.h>
#include <abscommon/Protocol.h>

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
