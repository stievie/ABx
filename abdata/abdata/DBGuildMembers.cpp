#include "stdafx.h"
#include "DBGuildMembers.h"
#include "Database.h"
#include "Utils.h"

namespace DB {

bool DBGuildMembers::Create(AB::Entities::GuildMembers& g)
{
    if (g.uuid.empty() || uuids::uuid(g.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Load(AB::Entities::GuildMembers& g)
{
    if (g.uuid.empty() || uuids::uuid(g.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    g.members.clear();

    std::ostringstream query;
    query << "SELECT * FROM `guild_members` WHERE ";
    query << "`guild_uuid` = " << db->EscapeString(g.uuid);
    query << " AND (`expires` = 0 OR `expires` > " << Utils::AbTick() << ")";

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
    if (g.uuid.empty() || uuids::uuid(g.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Delete(const AB::Entities::GuildMembers& g)
{
    if (g.uuid.empty() || uuids::uuid(g.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Exists(const AB::Entities::GuildMembers& g)
{
    if (g.uuid.empty() || uuids::uuid(g.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
