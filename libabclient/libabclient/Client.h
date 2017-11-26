#pragma once

#include <string>
#include <stdint.h>
#include <memory>
#include "Account.h"

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
    void OnGetCharlist();
    void OnEnterWorld();
public:
    Client();
    ~Client();
    void Login(const std::string& name, const std::string& pass);
    void EnterWorld(const std::string& charName);

    std::string loginHost_;
    uint16_t loginPort_;
    ClientState state_;
    const Charlist& GetCharacters() const;
};

}
