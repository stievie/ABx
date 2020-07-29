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


#include "DBIpBan.h"

namespace DB {

bool DBIpBan::Create(AB::Entities::IpBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream dbQuery;
    dbQuery << "SELECT COUNT(*) as count FROM ip_bans WHERE ";
    dbQuery << "((" << ban.ip << " & " << ban.mask << " & mask) = (ip & mask & " << ban.mask << "))";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(dbQuery.str());
    if (result && result->GetInt("count") != 0)
    {
        LOG_ERROR << "There is already a record matching this IP and mask" << std::endl;
        return false;
    }

    std::ostringstream query;
    query << "INSERT INTO ip_bans (uuid, ban_uuid, ip, mask) VALUES (";
    query << db->EscapeString(ban.uuid) << ", ";
    query << db->EscapeString(ban.banUuid) << ", ";
    query << ban.ip << ", ";
    query << ban.mask;

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    return transaction.Commit();
}

bool DBIpBan::Load(AB::Entities::IpBan& ban)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM ip_bans WHERE ";
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        query << "uuid = " << ban.uuid;
    else if (ban.ip != 0)
    {
        if (ban.mask == 0)
        {
            LOG_ERROR << "IP mask is 0 it would match all IPs" << std::endl;
            return false;
        }
        query << "((" << ban.ip << " & " << ban.mask << " & mask) = (ip & mask & " << ban.mask << "))";
    }
    else
    {
        LOG_ERROR << "UUID and IP are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ban.uuid = result->GetString("uuid");
    ban.banUuid = result->GetString("ban_uuid");
    ban.ip = result->GetUInt("ip");
    ban.mask = result->GetUInt("mask");

    return true;
}

bool DBIpBan::Save(const AB::Entities::IpBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE ip_bans SET ";
    query << " ban_uuid" << db->EscapeString(ban.banUuid) << ", ";
    query << " ip" << ban.ip << ", ";
    query << " mask" << ban.ip;

    query << " WHERE uuid = " << db->EscapeString(ban.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    return transaction.Commit();
}

bool DBIpBan::Delete(const AB::Entities::IpBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM ip_bans WHERE uuid = " << db->EscapeString(ban.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    return transaction.Commit();
}

bool DBIpBan::Exists(const AB::Entities::IpBan& ban)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS count FROM ip_bans WHERE ";
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        query << "uuid = " << db->EscapeString(ban.uuid);
    else if (ban.ip != 0)
    {
        if (ban.mask == 0)
        {
            LOG_ERROR << "IP mask is 0 it would match all IPs" << std::endl;
            return false;
        }
        query << "((" << ban.ip << " & " << ban.mask << " & mask) = (ip & mask & " << ban.mask << "))";
    }
    else
    {
        LOG_ERROR << "UUID and IP are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
