#pragma once

#include <stdint.h>
#include <mutex>

class Bans
{
private:
    std::recursive_mutex lock;
public:
    Bans() = default;
    ~Bans() {}

    bool AcceptConnection(uint32_t clientIP);

    static Bans Instance;
};

