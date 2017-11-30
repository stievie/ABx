#pragma once

#include "Protocol.h"
#include <string>
#include <stdint.h>

namespace Client {

enum GameServerOpCodes : uint8_t
{
    GameServerLoginOrPendingState = 10,
    GameServerGMActions = 11,
    GameServerEnterGame = 15,
    GameServerUpdateNeeded = 17,
    GameServerLoginError = 20,
    GameServerLoginAdvice = 21,
    GameServerLoginWait = 22,
    GameServerLoginSuccess = 23,
    GameServerLoginToken = 24,
    GameServerStoreButtonIndicators = 25, // 1097
    GameServerPingBack = 29,
    GameServerPing = 30,
    GameServerChallenge = 31,
    GameServerDeath = 40,
};

class ProtocolGame : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
    typedef std::function<void(const std::string& mapName)> EnterWorldCallback;
private:
    std::string accountName_;
    std::string accountPass_;
    std::string charName_;
    std::string map_;
    bool firstRevc_;
    EnterWorldCallback enterWorldCallback_;

    void SendLoginPacket();
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
    void OnError(const asio::error_code& err) override;

    void ParseMessage(const std::shared_ptr<InputMessage>& message);
    void ParseEnterWorld(const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame();

    void Login(const std::string& accountName, const std::string& accountPass,
        const std::string& charName, const std::string& map, const std::string& host, uint16_t port,
        const EnterWorldCallback& callback);
};

}
