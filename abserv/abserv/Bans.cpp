#include "stdafx.h"
#include "Bans.h"
#include "Utils.h"
#include "ConfigManager.h"
#include "Database.h"
#include "Utils.h"

#include "DebugNew.h"

namespace Auth {

BanManager BanManager::Instance;

bool BanManager::AcceptConnection(uint32_t clientIP)
{
    if (clientIP == 0)
        return false;

    lock_.lock();

    uint64_t currentTime = Utils::AbTick();
    std::map<uint32_t, ConnectBlock>::iterator it = ipConnects_.find(clientIP);
    if (it == ipConnects_.end())
    {
        ipConnects_.emplace(clientIP, ConnectBlock(currentTime, 0, 1));
        lock_.unlock();
        return true;
    }

    ConnectBlock& cb = it->second;
    cb.count++;
    if (cb.blockTime > currentTime)
    {
        lock_.unlock();
        return false;
    }

    if (currentTime - cb.startTime > 1000)
    {
        uint32_t connectionsPerSec = cb.count;
        cb.startTime = currentTime;
        cb.count = 0;
        cb.blockTime = 0;

        if (connectionsPerSec > 10)
        {
            cb.blockTime = currentTime + 10000;
            lock_.unlock();
            return false;
        }
    }

    lock_.unlock();
    return true;
}

bool BanManager::IsIpBanned(uint32_t clientIP, uint32_t mask /* = 0xFFFFFFFF */)
{
    if (clientIP == 0)
        return false;

    DB::Database* db = DB::Database::Instance();
    DB::DBQuery query;
    query << "SELECT COUNT(ip_bans.*) AS `count` FROM `ip_bans` "
        "INNER JOIN `bans` ON `bans`.`id` = `ip_bans`.`ban_id` "
        "WHERE " << "((" << clientIP << " & " << mask << " & `mask`) = (`ip` & `mask` & " << mask << ")) "
        "AND `active` = 1 AND (`expires` >= " << (Utils::AbTick() / 1000) << " OR `expires` <= 0)";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    return result->GetInt("count") > 0;
}

bool BanManager::IsIpDisabled(uint32_t clientIP)
{
    if (clientIP == 0)
        return false;

    uint32_t loginTries = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::LoginTries].GetInt());
    if (loginTries == 0)
        return false;

    std::lock_guard<std::recursive_mutex> lockGuard(lock_);
    std::map<uint32_t, LoginBlock>::iterator it = ipLogins_.find(clientIP);
    if (it == ipLogins_.end())
        return false;

    time_t currentTime = (Utils::AbTick() / 1000);

    uint32_t loginTimeout = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::LoginTimeout].GetInt()) / 1000;
    if ((it->second.numberOfLogins >= loginTries) &&
        (uint32_t)currentTime < it->second.lastLoginTime + loginTimeout)
        return true;

    return false;
}

bool BanManager::IsAccountBanned(uint32_t accountId)
{
    DB::Database* db = DB::Database::Instance();
    DB::DBQuery query;
    query <<
        "SELECT COUNT(*) as `count` "
        "FROM `account_bans` "
        "INNER JOIN `bans` ON `bans`.`id` = `account_bans`.`ban_id` "
        "WHERE "
        "`account_id` = " << accountId << " AND `active` = 1 AND (`expires` >= " << (Utils::AbTick() / 1000) << " OR `expires` <= 0)";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    return result->GetInt("count") > 0;
}

void BanManager::AddLoginAttempt(uint32_t clientIP, bool success)
{
    if (clientIP == 0)
        return;

    time_t currentTime = (Utils::AbTick() / 1000);
    std::lock_guard<std::recursive_mutex> lockGuard(lock_);
    std::map<uint32_t, LoginBlock>::iterator it = ipLogins_.find(clientIP);
    if (it == ipLogins_.end())
    {
        LoginBlock lb;
        lb.lastLoginTime = 0;
        lb.numberOfLogins = 0;
        ipLogins_[clientIP] = lb;
        it = ipLogins_.find(clientIP);
    }

    uint32_t loginTries = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::LoginTries].GetInt());
    if (it->second.numberOfLogins >= loginTries)
        it->second.numberOfLogins = 0;

    uint32_t retryTimeout = static_cast<uint32_t>(ConfigManager::Instance[ConfigManager::LoginRetryTimeout].GetInt()) / 1000;
    if (!success || (currentTime < it->second.lastLoginTime + retryTimeout))
    {
        ++it->second.numberOfLogins;
    }
    else
    {
        it->second.numberOfLogins = 0;
    }
    it->second.lastLoginTime = currentTime;
}

bool BanManager::AddIpBan(uint32_t ip, uint32_t mask, int32_t expires, uint32_t adminId,
    const std::string& comment, BanReason reason /* = ReasonOther */)
{
    if (ip == 0 || mask == 0)
        return false;

    DB::Database* db = DB::Database::Instance();
    DB::DBQuery query;
    DB::DBInsert stmt(db);

    stmt.SetQuery("INSERT INTO `bans` (`expires`, `added`, `reason`, `active`, `admin_id`, `comment`) VALUES ");
    query << expires << ", " << (Utils::AbTick() / 1000) << reason << ", 1, " << adminId << ", " << db->EscapeString(comment);

    if (!stmt.AddRow(query.str()))
        return false;
    if (!stmt.Execute())
        return false;

    uint64_t banId = db->GetLastInsertId();

    stmt.SetQuery("INSERT INTO `ip_bans` (`ban_id`, `ip`, `mask`) VALUES ");
    query.Reset();
    query << banId << ", " << ip << ", " << mask;

    if (!stmt.AddRow(query.str()))
        return false;
    if (!stmt.Execute())
        return false;

    return true;
}

bool BanManager::AddAccountBan(uint32_t accountId, int32_t expires, uint32_t adminId,
    const std::string & comment, BanReason reason /* = ReasonOther */)
{
    if (accountId == 0)
        return false;

    DB::Database* db = DB::Database::Instance();
    DB::DBQuery query;
    DB::DBInsert stmt(db);

    stmt.SetQuery("INSERT INTO `bans` (`expires`, `added`, `reason`, `active`, `admin_id`, `comment`) VALUES ");
    query << expires << ", " << (Utils::AbTick() / 1000) << reason << ", 1, " << adminId << ", " << db->EscapeString(comment);

    if (!stmt.AddRow(query.str()))
        return false;
    if (!stmt.Execute())
        return false;

    uint64_t banId = db->GetLastInsertId();

    stmt.SetQuery("INSERT INTO `account_bans` (`ban_id`, `account_id`) VALUES ");
    query.Reset();
    query << banId << ", " << accountId;

    if (!stmt.AddRow(query.str()))
        return false;
    if (!stmt.Execute())
        return false;

    return true;
}

}
