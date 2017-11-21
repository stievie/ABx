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
    std::unique_ptr<ProtocolGame> proto_;
public:
    Client();
    ~Client();
    void Login(const std::string& name, const std::string& pass);

    std::string loginHost_;
    uint16_t loginPort_;
};

}
