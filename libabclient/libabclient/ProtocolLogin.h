#pragma once

#include "Protocol.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/Service.h>
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>
#include "Structs.h"

namespace Client {

class ProtocolLogin : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = AB::ProtocolLoginId };
    enum { UseChecksum = true };
    typedef std::function<void(const std::string& accountUuid)> LoggedInCallback;
    typedef std::function<void(const AB::Entities::CharList& chars)> CharlistCallback;
    typedef std::function<void(const std::vector<AB::Entities::Game>& outposts)> GamelistCallback;
    typedef std::function<void(const std::vector<AB::Entities::Service>& services)> ServerlistCallback;
    typedef std::function<void()> CreateAccountCallback;
    typedef std::function<void(const std::string& uuid, const std::string& mapUuid)> CreatePlayerCallback;
private:
    enum ProtocolAction
    {
        ActionUnknown,
        ActionLogin,
        ActionCreateAccount,
        ActionCreatePlayer,
        ActionGetOutposts,
        ActionGetServers,
    };
    ProtocolAction action_;
    std::string host_;
    uint16_t port_;
    std::string accountName_;
    std::string accountUuid_;
    std::string password_;
    std::string email_;
    std::string accKey_;
    std::string charName_;
    std::string profUuid_;
    uint32_t itemIndex_;
    AB::Entities::CharacterSex sex_;
    bool isPvp_;
    bool firstRecv_;
    LoggedInCallback loggedInCallback_;
    CharlistCallback charlistCallback_;
    GamelistCallback gamelistCallback_;
    ServerlistCallback serverlistCallback_;
    CreateAccountCallback createAccCallback_;
    CreatePlayerCallback createPlayerCallback_;
    void SendLoginPacket();
    void SendCreateAccountPacket();
    void SendCreatePlayerPacket();
    void SendGetOutpostsPacket();
    void SendGetServersPacket();
    void ParseMessage(const std::shared_ptr<InputMessage>& message);
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
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
        const std::string& accountUuid, const std::string& password,
        const std::string& charName, const std::string& profUuid,
        uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp,
        const CreatePlayerCallback& callback);
    void GetOutposts(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& password,
        const GamelistCallback& callback);
    void GetServers(std::string& host, uint16_t port,
        const std::string& accountUuid, const std::string& password,
        const ServerlistCallback& callback);

    std::string gameHost_;
    uint16_t gamePort_;
    std::string fileHost_;
    uint16_t filePort_;
};

}
