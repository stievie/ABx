#pragma once

#include <string>
#include <stdint.h>
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include <memory>

namespace Client {

class Client : public std::enable_shared_from_this<Client>
{
public:
    enum ClientState
    {
        StateDisconnected,
        StateSelecChar,
        StateWorld
    };
private:
    std::shared_ptr<ProtocolLogin> protoLogin_;
    std::shared_ptr<ProtocolGame> protoGame_;
    std::string accountName_;
    std::string password_;
    void OnGetCharlist();
public:
    Client();
    ~Client();
    void Login(const std::string& name, const std::string& pass);
    void EnterWorld(const std::string& charName);

    std::string loginHost_;
    uint16_t loginPort_;
    ClientState state_;
    const Charlist& GetCharacters() const
    {
        static Charlist empty;
        if (!protoLogin_)
            return empty;
        return protoLogin_->characters_;
    }
};

}
