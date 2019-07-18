#include "stdafx.h"
#include "BanManager.h"
#include "Utils.h"
#include <AB/Entities/IpBan.h>
#include <AB/Entities/AccountBan.h>
#include "DataClient.h"
#include "Profiler.h"
#include "Subsystems.h"

namespace Auth {

uint32_t BanManager::LoginTries = 5;
uint32_t BanManager::LoginRetryTimeout = 5000;

bool BanManager::AcceptConnection(uint32_t clientIP)
{
    if (clientIP == 0)
        return false;

    int64_t currentTime = Utils::Tick();
    std::map<uint32_t, ConnectBlock>::iterator it = ipConnects_.find(clientIP);
    if (it == ipConnects_.end())
    {
        std::lock_guard<std::mutex> lock(lock_);
        ipConnects_.emplace(clientIP, ConnectBlock(currentTime, 0, 1));
        return true;
    }

    ConnectBlock& cb = it->second;
    {
        std::lock_guard<std::mutex> lock(lock_);
        ++cb.count;
    }
    if (cb.blockTime > currentTime)
        return false;

    if (currentTime - cb.startTime > 1000)
    {
        std::lock_guard<std::mutex> lock(lock_);
        uint32_t connectionsPerSec = cb.count;
        cb.startTime = currentTime;
        cb.count = 0;
        cb.blockTime = 0;

        if (connectionsPerSec > 10)
        {
            cb.blockTime = currentTime + 10000;
            return false;
        }
    }

    return true;
}

bool BanManager::IsIpBanned(uint32_t clientIP, uint32_t mask /* = 0xFFFFFFFF */)
{
    if (clientIP == 0)
        return false;

    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client)
        return false;

    AB::Entities::IpBan ban;
    ban.ip = clientIP;
    ban.mask = mask;
    if (!client->Read(ban))
        return false;
    AB::Entities::Ban _ban;
    _ban.uuid = ban.banUuid;
    if (!client->Read(_ban))
        return false;
    if (!_ban.active)
        return false;
    return (_ban.expires <= 0) || (_ban.expires >= Utils::Tick() / 1000);
}

bool BanManager::IsIpDisabled(uint32_t clientIP)
{
    if (clientIP == 0)
        return false;

    if (BanManager::LoginTries == 0)
        return false;

    std::map<uint32_t, LoginBlock>::iterator it = ipLogins_.find(clientIP);
    if (it == ipLogins_.end())
        return false;

    time_t currentTime = (Utils::Tick() / 1000);

    if ((it->second.numberOfLogins >= BanManager::LoginTries) &&
        currentTime < it->second.lastLoginTime + BanManager::LoginRetryTimeout)
        return true;

    return false;
}

bool BanManager::IsAccountBanned(const uuids::uuid& accountUuid)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client)
        return false;

    AB::Entities::AccountBan ban;
    ban.accountUuid = accountUuid.to_string();
    if (!client->Read(ban))
        return false;
    AB::Entities::Ban _ban;
    _ban.uuid = ban.banUuid;
    if (!client->Read(_ban))
        return false;
    if (!_ban.active)
        return false;
    return (_ban.expires <= 0) || (_ban.expires >= Utils::Tick() / 1000);
}

void BanManager::AddLoginAttempt(uint32_t clientIP, bool success)
{
    if (clientIP == 0)
        return;

    time_t currentTime = (Utils::Tick() / 1000);
    std::lock_guard<std::mutex> lockGuard(lock_);
    std::map<uint32_t, LoginBlock>::iterator it = ipLogins_.find(clientIP);
    if (it == ipLogins_.end())
    {
        LoginBlock lb{ 0, 0 };
        ipLogins_[clientIP] = lb;
        it = ipLogins_.find(clientIP);
    }

    if (it->second.numberOfLogins >= BanManager::LoginTries)
        it->second.numberOfLogins = 0;

    if (!success || (currentTime < it->second.lastLoginTime + BanManager::LoginRetryTimeout))
        ++it->second.numberOfLogins;
    else
        it->second.numberOfLogins = 0;
    it->second.lastLoginTime = currentTime;
}

bool BanManager::AddIpBan(uint32_t ip, uint32_t mask, int32_t expires, const std::string& adminUuid,
    const std::string& comment, AB::Entities::BanReason reason /* = AB::Entities::BanReasonOther */)
{
    if (ip == 0 || mask == 0)
        return false;

    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client)
        return false;

    AB::Entities::Ban ban;
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    ban.uuid = guid.to_string();
    ban.expires = expires;
    ban.added = (Utils::Tick() / 1000);
    ban.reason = reason;
    ban.adminUuid = adminUuid;
    ban.comment = comment;
    if (!client->Create(ban))
        return false;

    AB::Entities::IpBan ipBan;
    ipBan.banUuid = guid.to_string();
    ipBan.ip = ip;
    ipBan.mask = mask;
    return client->Create(ipBan);
}

bool BanManager::AddAccountBan(const std::string& accountUuid, int32_t expires, const std::string& adminUuid,
    const std::string& comment, AB::Entities::BanReason reason /* = AB::Entities::BanReasonOther */)
{
    if (accountUuid.empty() || uuids::uuid(accountUuid).nil())
        return false;

    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Ban ban;
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    ban.uuid = guid.to_string();
    ban.expires = expires;
    ban.added = (Utils::Tick() / 1000);
    ban.reason = reason;
    ban.adminUuid = adminUuid;
    ban.comment = comment;
    if (!client->Create(ban))
        return false;

    AB::Entities::AccountBan accBan;
    accBan.banUuid = guid.to_string();
    accBan.accountUuid = accountUuid;
    return client->Create(accBan);
}

}
