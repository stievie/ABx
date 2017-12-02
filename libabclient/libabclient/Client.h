#pragma once

#include <string>
#include <stdint.h>
#include <memory>
#include "Account.h"
#include "Receiver.h"

namespace Client {

class ProtocolLogin;
class ProtocolGame;

class Client
{
public:
    enum ClientState
    {
        StateDisconnected,
        StateCreateChar,
        StateSelecChar,
        StateWorld
    };
private:
    std::shared_ptr<ProtocolLogin> protoLogin_;
    std::shared_ptr<ProtocolGame> protoGame_;
    std::string accountName_;
    std::string password_;
    std::string mapName_;
    void OnGetCharlist(const Charlist& chars);
    void OnEnterWorld(const std::string& mapName);
    void OnError(const std::error_code& err);
    void OnProtocolError(uint8_t err);
public:
    Client();
    ~Client();
    /// Login to login server
    void Login(const std::string& name, const std::string& pass);
    void Logout();
    /// Connect to game server -> authenticate -> enter game
    void EnterWorld(const std::string& charName, const std::string& map);
    void Update();

    std::string loginHost_;
    uint16_t loginPort_;
    std::string gameHost_;
    uint16_t gamePort_;
    ClientState state_;
    Receiver* receiver_;
    const std::string& GetMapName() const
    {
        return mapName_;
    }
};

}
