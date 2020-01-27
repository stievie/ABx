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

#include "stdafx.h"
#include "DBAccountBan.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBAccountBan::Create(AB::Entities::AccountBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `account_bans` (`uuid`, `ban_uuid`, `account_uuid`) VALUES (";
    query << db->EscapeString(ban.uuid) << ", ";
    query << db->EscapeString(ban.banUuid) << ", ";
    query << db->EscapeString(ban.accountUuid);

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBAccountBan::Load(AB::Entities::AccountBan& ban)
{
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "SELECT * FROM `account_bans` WHERE ";
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        query << "`uuid` = " << ban.uuid;
    else if (!ban.accountUuid.empty() && !uuids::uuid(ban.accountUuid).nil())
        query << "`account_uuid` = " << db->EscapeString(ban.accountUuid);
    else
    {
        LOG_ERROR << "UUID and Account UUID are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ban.uuid = result->GetString("uuid");
    ban.banUuid = result->GetString("ban_uuid");
    ban.accountUuid = result->GetString("account_uuid");

    return true;
}

bool DBAccountBan::Save(const AB::Entities::AccountBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `account_bans` SET ";
    query << " `ban_uuid`" << ban.banUuid << ", ";
    query << " `account_uuid`" << ban.accountUuid;

    query << " WHERE `uuid` = " << db->EscapeString(ban.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccountBan::Delete(const AB::Entities::AccountBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `account_bans` WHERE `uuid` = " << db->EscapeString(ban.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccountBan::Exists(const AB::Entities::AccountBan& ban)
{
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `account_bans` WHERE ";
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        query << "`uuid` = " << db->EscapeString(ban.uuid);
    else if (!ban.accountUuid.empty() && !uuids::uuid(ban.accountUuid).nil())
        query << "`account_uuid` = " << db->EscapeString(ban.accountUuid);
    else
    {
        LOG_ERROR << "UUID and Account UUID are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    return result->GetUInt("count") != 0;
}

}
