#pragma once

#include <system_error>
#include <stdint.h>

namespace Client {

class Receiver
{
public:
    virtual void OnGetCharlist() = 0;
    virtual void OnEnterWorld() = 0;
    virtual void OnNetworkError(const std::error_code& err) = 0;
    virtual void OnProtocolError(uint8_t err) = 0;
};

}
