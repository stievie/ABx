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
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::FriendList& fl, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "account_uuid")
            return db->EscapeString(fl.uuid);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

static std::string PlaceholderCallbackFriend(Database* db, const AB::Entities::FriendList& fl, const AB::Entities::Friend& fr, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "account_uuid")
            return db->EscapeString(fl.uuid);
        if (token.value == "friend_uuid")
            return db->EscapeString(fr.friendUuid);
        if (token.value == "friend_name")
            return db->EscapeString(fr.friendName);
        if (token.value == "relation")
            return std::to_string(static_cast<int>(fr.relation));
        if (token.value == "creation")
            return std::to_string(fr.creation);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

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

    static constexpr const char* SQL = "SELECT * FROM friend_list WHERE account_uuid = ${account_uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, fl, std::placeholders::_1));

    fl.friends.clear();
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
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

bool DBFriendList::Save(const AB::Entities::FriendList& fl)
{
    if (Utils::Uuid::IsEmpty(fl.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    // First delete all
    static constexpr const char* SQL_DELETE = "DELETE FROM friend_list WHERE account_uuid = ${account_uuid}";
    const std::string deleteQuery = sa::templ::Parser::Evaluate(SQL_DELETE, std::bind(&PlaceholderCallback, db, fl, std::placeholders::_1));
    if (!db->ExecuteQuery(deleteQuery))
        return false;

    if (fl.friends.size() > 0)
    {
        static constexpr const char* SQL_INSERT = "INSERT INTO friend_list ("
                "account_uuid, friend_uuid, friend_name, relation, creation"
            ") VALUES ("
                "${account_uuid}, ${friend_uuid}, ${friend_name}, ${relation}, ${creation}"
            ")";
        sa::templ::Parser parser;
        const sa::templ::Tokens tokens = parser.Parse(SQL_INSERT);

        // Then add all
        for (const auto& f : fl.friends)
        {
            const std::string insertQuery = tokens.ToString(std::bind(&PlaceholderCallbackFriend, db, fl, f, std::placeholders::_1));
            if (!db->ExecuteQuery(insertQuery))
                return false;
        }
    }
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
    static constexpr const char* SQL = "DELETE FROM friend_list WHERE account_uuid = ${account_uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, fl, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

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
