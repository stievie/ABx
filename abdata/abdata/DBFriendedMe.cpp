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


#include "DBFriendedMe.h"

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
