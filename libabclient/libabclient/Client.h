#pragma once

#include <string>
#include <stdint.h>
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include <memory>

namespace Client {

class Client
{
private:
    std::shared_ptr<ProtocolLogin> protoLogin_;
    std::shared_ptr<ProtocolGame> protoGame_;
    std::string accountName_;;
    std::string password_;
public:
    Client();
    ~Client();
    void Login(const std::string& name, const std::string& pass);
    void EnterWorld(const std::string& charName);

    std::string loginHost_;
    uint16_t loginPort_;
};

}
