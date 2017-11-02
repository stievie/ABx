#pragma once

#include <stdint.h>
#include <mutex>
#include <map>

namespace Auth {

enum BanReason : uint8_t
{
    ReasonOther = 0,
    ReasonNameReport = 1,
    ReasonLeeching = 2,
    ReasonBotting = 3,
    ReasonScamming = 4
};

struct BanInfo
{
    std::string bannedBy;
    std::string comment;
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
    /// mask = network mask
    bool IsIpBanned(uint32_t clienttIP, uint32_t mask = 0xFFFFFFFF);
    /// May happen when there are too many connections from this IP
    bool IsIpDisabled(uint32_t clientIP);
    bool IsAccountBanned(uint32_t accountId);
    void AddLoginAttempt(uint32_t clientIP, bool success);
    bool AddIpBan(uint32_t ip, uint32_t mask, int32_t expires, uint32_t adminId, const std::string& comment,
        BanReason reason = ReasonOther);
    bool AddAccountBan(uint32_t accountId, int32_t expires, uint32_t adminId, const std::string& comment,
        BanReason reason = ReasonOther);

    static BanManager Instance;
};

}
