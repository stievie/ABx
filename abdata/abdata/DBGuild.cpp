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
#include "DBGuild.h"

namespace DB {

bool DBGuild::Create(AB::Entities::Guild& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `guilds` (`uuid`, `name`, `tag`, `creator_account_uuid`, `creation`, `guild_hall_uuid`, `creator_name`, `creator_player_uuid`, ";
    query << "`guild_hall_instance_uuid`, `guild_hall_server_uuid`";
    query << ") VALUES (";

    query << db->EscapeString(g.uuid) << ", ";
    query << db->EscapeString(g.name) << ", ";
    query << db->EscapeString(g.tag) << ", ";
    query << db->EscapeString(g.creatorAccountUuid) << ", ";
    query << g.creation << ", ";
    query << db->EscapeString(g.guildHall) << ", ";
    query << db->EscapeString(g.creatorName) << ", ";
    query << db->EscapeString(g.creatorPlayerUuid) << ", ";
    query << db->EscapeString(g.guildHallInstanceUuid) << ", ";
    query << db->EscapeString(g.guildHallServerUuid);

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

bool DBGuild::Load(AB::Entities::Guild& g)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `guilds` WHERE ";
    if (!Utils::Uuid::IsEmpty(g.uuid))
        query << "`uuid` = " << db->EscapeString(g.uuid);
    else if (!g.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(g.name) << ")";
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    g.uuid = result->GetString("uuid");
    g.name = result->GetString("name");
    g.tag = result->GetString("tag");
    g.creatorAccountUuid = result->GetString("creator_account_uuid");
    g.creation = result->GetLong("creation");
    g.guildHall = result->GetString("guild_hall_uuid");
    g.creatorName = result->GetString("creator_name");
    g.creatorPlayerUuid = result->GetString("creator_player_uuid");
    g.guildHallInstanceUuid = result->GetString("guild_hall_instance_uuid");
    g.guildHallServerUuid = result->GetString("guild_hall_server_uuid");

    return true;
}

bool DBGuild::Save(const AB::Entities::Guild& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `guilds` SET ";

    // Only these may be changed
    query << " `name` = " << db->EscapeString(g.name) << ", ";
    query << " `tag` = " << db->EscapeString(g.tag) << ", ";
    query << " `creator_account_uuid` = " << db->EscapeString(g.creatorAccountUuid) << ", ";
    query << " `creation` = " << g.creation << ", ";
    query << " `guild_hall_uuid` = " << db->EscapeString(g.guildHall) << ", ";
    query << " `creator_name` = " << db->EscapeString(g.creatorName) << ", ";
    query << " `creator_player_uuid` = " << db->EscapeString(g.creatorPlayerUuid) << ", ";
    query << " `guild_hall_instance_uuid` = " << db->EscapeString(g.guildHallInstanceUuid) << ", ";
    query << " `guild_hall_server_uuid` = " << db->EscapeString(g.guildHallServerUuid);

    query << " WHERE `uuid` = " << db->EscapeString(g.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBGuild::Delete(const AB::Entities::Guild& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `guilds` WHERE `uuid` = " << db->EscapeString(g.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBGuild::Exists(const AB::Entities::Guild& g)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `guilds` WHERE ";
    if (!Utils::Uuid::IsEmpty(g.uuid))
        query << "`uuid` = " << db->EscapeString(g.uuid);
    else if (!g.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(g.name) << ")";
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
