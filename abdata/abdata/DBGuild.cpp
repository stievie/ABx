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

#include "DBGuild.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Guild& g, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(g.uuid);
        if (token.value == "name")
            return db->EscapeString(g.name);
        if (token.value == "tag")
            return db->EscapeString(g.tag);
        if (token.value == "creator_account_uuid")
            return db->EscapeString(g.creatorAccountUuid);
        if (token.value == "creation")
            return std::to_string(g.creation);
        if (token.value == "guild_hall_uuid")
            return db->EscapeString(g.guildHall);
        if (token.value == "creator_name")
            return db->EscapeString(g.creatorName);
        if (token.value == "creator_player_uuid")
            return db->EscapeString(g.creatorPlayerUuid);
        if (token.value == "guild_hall_instance_uuid")
            return db->EscapeString(g.guildHallInstanceUuid);
        if (token.value == "guild_hall_server_uuid")
            return db->EscapeString(g.guildHallServerUuid);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBGuild::Create(AB::Entities::Guild& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "INSERT INTO guilds ("
            "uuid, name, tag, creator_account_uuid, creation, guild_hall_uuid, "
            "creator_name, creator_player_uuid, guild_hall_instance_uuid, guild_hall_server_uuid"
        ") VALUES ("
            "${uuid}, ${name}, ${tag}, ${creator_account_uuid}, ${creation}, ${guild_hall_uuid}, "
            "${creator_name}, ${creator_player_uuid}, ${guild_hall_instance_uuid}, ${guild_hall_server_uuid}"
        ")";

    Database* db = GetSubsystem<Database>();

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));
    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBGuild::Load(AB::Entities::Guild& g)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT * FROM guilds WHERE uuid= ${uuid}";
    static constexpr const char* SQL_NAME = "SELECT * FROM guilds WHERE LOWER(name) = LOWER(${name})";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(g.uuid))
        sql = SQL_UUID;
    else if (!g.name.empty())
        sql = SQL_NAME;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
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

    static constexpr const char* SQL = "UPDATE guilds SET "
        "name = ${name}, "
        "tag = ${tag}, "
        "creator_account_uuid = ${creator_account_uuid}, "
        "creation = ${creation}, "
        "guild_hall_uuid = ${guild_hall_uuid}, "
        "creator_name = ${creator_name}, "
        "creator_player_uuid = ${creator_player_uuid}, "
        "guild_hall_instance_uuid = ${guild_hall_instance_uuid}, "
        "guild_hall_server_uuid = ${guild_hall_server_uuid} "
        "WHERE uuid = ${uuid}";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

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

    static constexpr const char* SQL = "DELETE FROM guilds WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBGuild::Exists(const AB::Entities::Guild& g)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT COUNT(*) AS count FROM guilds WHERE uuid= ${uuid}";
    static constexpr const char* SQL_NAME = "SELECT COUNT(*) AS count FROM guilds WHERE LOWER(name) = LOWER(${name})";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(g.uuid))
        sql = SQL_UUID;
    else if (!g.name.empty())
        sql = SQL_NAME;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
