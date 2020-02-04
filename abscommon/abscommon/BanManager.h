/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <AB/Entities/Ban.h>
#include <map>
#include <uuid.h>
#include <mutex>

namespace Auth {

struct BanInfo
{
    std::string bannedBy;
    std::string comment;
    time_t expiresAt;
};

struct LoginBlock
{
    int64_t lastLoginTime;
    uint32_t numberOfLogins;
};

struct ConnectBlock
{
    constexpr ConnectBlock(int64_t startTime, int64_t blockTime, uint32_t count) :
        startTime(startTime),
        blockTime(blockTime),
        count(count)
    {}
    int64_t startTime;
    int64_t blockTime;
    uint32_t count;
};

class BanManager
{
private:
    std::mutex lock_;
    std::map<uint32_t, LoginBlock> ipLogins_;
    std::map<uint32_t, ConnectBlock> ipConnects_;
public:
    static uint32_t LoginTries;
    static uint32_t LoginRetryTimeout;

    BanManager() = default;
    ~BanManager() {}

    // Avoid brute forcing logins
    bool IsIpDisabled(uint32_t clientIP) const;
    void AddLoginAttempt(uint32_t clientIP, bool success);

    // Avoid DOS
    bool AcceptConnection(uint32_t clientIP);

    // Bans
    /// mask = network mask
    bool IsIpBanned(uint32_t clientIP, uint32_t mask = 0xFFFFFFFF) const;
    /// May happen when there are too many connections from this IP
    bool IsAccountBanned(const uuids::uuid& accountUuid);
    bool AddIpBan(uint32_t ip, uint32_t mask, int64_t expires,
        const std::string& adminUuid, const std::string& comment,
        AB::Entities::BanReason reason = AB::Entities::BanReasonOther);
    bool AddAccountBan(const std::string& accountUuid, int64_t expires,
        const std::string& adminUuid, const std::string& comment,
        AB::Entities::BanReason reason = AB::Entities::BanReasonOther);
};

}
