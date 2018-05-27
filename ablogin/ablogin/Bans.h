#pragma once

#include <AB/Entities/Ban.h>

namespace Auth {

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
    bool IsAccountBanned(const uuids::uuid& accountUuid);
    void AddLoginAttempt(uint32_t clientIP, bool success);
    bool AddIpBan(uint32_t ip, uint32_t mask, int32_t expires, const std::string& adminUuid, const std::string& comment,
        AB::Entities::BanReason reason = AB::Entities::BanReasonOther);
    bool AddAccountBan(const std::string& accountUuid, int32_t expires, const std::string& adminUuid, const std::string& comment,
        AB::Entities::BanReason reason = AB::Entities::BanReasonOther);

    uint32_t loginTries_;
    uint32_t loginTimeout_;
    uint32_t retryTimeout_;

    static BanManager Instance;
};

}
