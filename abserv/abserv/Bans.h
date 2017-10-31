#pragma once

#include <stdint.h>
#include <mutex>
#include <map>

namespace Auth {

struct BanInfo
{
    std::string bannedBy;
    std::string reason;
    time_t expiresAt;
};
struct LoginBlock
{
    time_t lastLoginTime;
    uint32_t numberOfLogins;
};

struct ConnectBlock
{
    constexpr ConnectBlock(uint64_t startTime, uint64_t blockTime, uint32_t count) :
        startTime(startTime),
        blockTime(blockTime),
        count(count)
    {}
    uint64_t startTime;
    uint64_t blockTime;
    uint32_t count;
};

class BanManager
{
private:
    std::recursive_mutex lock_;
    std::map<uint32_t, LoginBlock> ipLogins_;
    std::map<uint32_t, ConnectBlock> ipConnects_;
public:
    BanManager() = default;
    ~BanManager() {}

    bool AcceptConnection(uint32_t clientIP);
    bool IsIpBanned(uint32_t clienttIP, BanInfo& info);

    static BanManager Instance;
};

}
