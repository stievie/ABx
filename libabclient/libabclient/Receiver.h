#pragma once

#include <system_error>
#include <stdint.h>
#include <string>
#include "Account.h"

namespace Client {

class Receiver
{
public:
    virtual void OnGetCharlist(const Charlist& chars) = 0;
    virtual void OnEnterWorld(const std::string& mapName) = 0;
    virtual void OnNetworkError(const std::error_code& err) = 0;
    virtual void OnProtocolError(uint8_t err) = 0;
};

}
