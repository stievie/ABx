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


#include "DBGuildMembers.h"
#include "StorageProvider.h"

namespace DB {

bool DBGuildMembers::Create(AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Load(AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    g.members.clear();

    std::ostringstream query;
    query << "SELECT * FROM guild_members WHERE ";
    query << "guild_uuid = " << db->EscapeString(g.uuid);
    query << " AND (expires = 0 OR expires > " << Utils::Tick() << ")";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        // No members
        return true;

    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        AB::Entities::GuildMember gm;
        gm.accountUuid = result->GetString("account_uuid");
        gm.inviteName = result->GetString("invite_name");
        gm.role = static_cast<AB::Entities::GuildRole>(result->GetUInt("role"));
        gm.invited = result->GetLong("invited");
        gm.joined = result->GetLong("joined");
        gm.expires = result->GetLong("expires");
        g.members.push_back(gm);
    }

    return true;
}

bool DBGuildMembers::Save(const AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Delete(const AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Exists(const AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

void DBGuildMembers::DeleteExpired(StorageProvider* sp)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT guild_uuid FROM guild_members WHERE ";
    query << "(expires <> 0 AND expires < " << Utils::Tick() << ")";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        // No members
        return;

    std::vector<std::string> guilds;
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        guilds.push_back(result->GetString("guild_uuid"));
    }

    query.str("");
    query << "DELETE FROM guild_members WHERE ";
    query << "(expires <> 0 AND expires < " << Utils::Tick() << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return;

    if (!db->ExecuteQuery(query.str()))
        return;

    // End transaction
    if (!transaction.Commit())
        return;

    for (const std::string& g : guilds)
    {
        AB::Entities::GuildMembers gms;
        gms.uuid = g;
        sp->EntityInvalidate(gms);
    }
}

}
