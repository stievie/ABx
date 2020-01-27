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

#include "Protocol.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/Service.h>
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>
#include "Structs.h"
#include <AB/Packets/LoginPackets.h>

namespace Client {

class ProtocolLogin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = AB::ProtocolLoginId };
    enum { UseChecksum = true };
    typedef std::function<void(const std::string& accountUuid, const std::string& authToken)> LoggedInCallback;
    typedef std::function<void(const AB::Entities::CharList& chars)> CharlistCallback;
    typedef std::function<void(const std::vector<AB::Entities::Game>& outposts)> GamelistCallback;
    typedef std::function<void(const std::vector<AB::Entities::Service>& services)> ServerlistCallback;
    typedef std::function<void()> CreateAccountCallback;
    typedef std::function<void(const std::string& uuid, const std::string& mapUuid)> CreatePlayerCallback;
    typedef std::function<void()> AccountKeyAddedCallback;
private:
    LoggedInCallback loggedInCallback_;
    CharlistCallback charlistCallback_;
    GamelistCallback gamelistCallback_;
    ServerlistCallback serverlistCallback_;
    CreateAccountCallback createAccCallback_;
    CreatePlayerCallback createPlayerCallback_;
    AccountKeyAddedCallback accountKeyAddedCallback_;
    bool firstRecv_;
    void ParseMessage(InputMessage& message);

    void HandleCharList(const AB::Packets::Server::Login::CharacterList& packet);
    void HandleOutpostList(const AB::Packets::Server::Login::OutpostList& packet);
    void HandleServerList(const AB::Packets::Server::Login::ServerList& packet);
    void HandleLoginError(const AB::Packets::Server::Login::Error& packet);
    void HandleCreatePlayerSuccess(const AB::Packets::Server::Login::CreateCharacterSuccess& packet);
protected:
    void OnReceive(InputMessage& message) override;
public:
    ProtocolLogin(Crypto::DHKeys& keys, asio::io_service& ioService);
    ~ProtocolLogin() override {}

    void Login(std::string& host, uint16_t port,
        const std::string& account, const std::string& password,
        const LoggedInCallback& onLoggedIn,
        const CharlistCallback& callback);
    void CreateAccount(std::string& host, uint16_t port,
        const std::string& account, const std::string& password,
        const std::string& email, const std::string& accKey,
        const CreateAccountCallback& callback);
    void CreatePlayer(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const std::string& charName, const std::string& profUuid,
        uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp,
        const CreatePlayerCallback& callback);
    void AddAccountKey(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const std::string& newAccountKey,
        const AccountKeyAddedCallback& callback);
    void GetOutposts(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const GamelistCallback& callback);
    void GetServers(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& token,
        const ServerlistCallback& callback);

    std::string gameHost_;
    uint16_t gamePort_;
    std::string fileHost_;
    uint16_t filePort_;
};

}
