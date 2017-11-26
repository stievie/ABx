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
    void OnGetCharlist();
    void OnEnterWorld();
    void OnError(const std::error_code& err);
    void OnProtocolError(uint8_t err);
public:
    Client();
    ~Client();
    void Login(const std::string& name, const std::string& pass);
    void EnterWorld(const std::string& charName);

    std::string loginHost_;
    uint16_t loginPort_;
    ClientState state_;
    Receiver* receiver_;
    const Charlist& GetCharacters() const;
};

}
