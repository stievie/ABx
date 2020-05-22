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


#include "DBFriendList.h"

namespace DB {

bool DBFriendList::Create(AB::Entities::FriendList& fl)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBFriendList::Load(AB::Entities::FriendList& fl)
{
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    fl.friends.clear();
    std::ostringstream query;
    query << "SELECT * FROM `friend_list` WHERE `account_uuid` = " << db->EscapeString(fl.uuid);
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (result)
    {
        for (result = db->StoreQuery(query.str()); result; result = result->Next())
        {
            fl.friends.push_back({
                result->GetString("friend_uuid"),
                result->GetString("friend_name"),
                static_cast<AB::Entities::FriendRelation>(result->GetUInt("relation")),
                result->GetLong("creation")
            });
        }
        return true;
    }

    return false;
}

bool DBFriendList::Save(const AB::Entities::FriendList& fl)
{
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    // First delete all
    query << "DELETE FROM `friend_list` WHERE `account_uuid` = " << db->EscapeString(fl.uuid);

    if (!db->ExecuteQuery(query.str()))
        return false;

    if (fl.friends.size() > 0)
    {
        // Then add all
        for (const auto& f : fl.friends)
        {
            query.str("");
            query << "INSERT INTO `friend_list` (`account_uuid`, `friend_uuid`, `friend_name`, `relation`, `creation`) VALUES (";
            query << db->EscapeString(fl.uuid) << ", ";
            query << db->EscapeString(f.friendUuid) << ", ";
            query << db->EscapeString(f.friendName) << ", ";
            query << static_cast<int>(f.relation) << ", ";
            query << f.creation;
            query << ")";
            if (!db->ExecuteQuery(query.str()))
                return false;
        }
    }
    // End transaction
    return transaction.Commit();
}

bool DBFriendList::Delete(const AB::Entities::FriendList& fl)
{
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    // Delete all friends of this account
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `friend_list` WHERE `account_uuid` = " << db->EscapeString(fl.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBFriendList::Exists(const AB::Entities::FriendList& fl)
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
