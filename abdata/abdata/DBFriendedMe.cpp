#include "stdafx.h"
#include "DBFriendedMe.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBFriendedMe::Create(AB::Entities::FriendedMe& fl)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBFriendedMe::Load(AB::Entities::FriendedMe& fl)
{
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    fl.friends.clear();
    std::ostringstream query;
    query << "SELECT * FROM `friend_list` WHERE `friend_uuid` = " << db->EscapeString(fl.uuid);
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (result)
    {
        for (result = db->StoreQuery(query.str()); result; result = result->Next())
        {
            fl.friends.push_back({
                result->GetString("account_uuid"),
                static_cast<AB::Entities::FriendRelation>(result->GetUInt("relation"))
            });
        }
        return true;
    }

    return false;
}

bool DBFriendedMe::Save(const AB::Entities::FriendedMe& fl)
{
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBFriendedMe::Delete(const AB::Entities::FriendedMe& fl)
{
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBFriendedMe::Exists(const AB::Entities::FriendedMe& fl)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

}
