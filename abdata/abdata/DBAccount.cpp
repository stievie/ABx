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

#include "DBAccount.h"
#include <sa/Assert.h>
#include <sa/TemplateParser.h>
#include <uuid.h>
#include <abscommon/Profiler.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Account& account, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(account.uuid);
        if (token.value == "name")
            return db->EscapeString(account.name);
        if (token.value == "password")
            return db->EscapeString(account.password);
        if (token.value == "email")
            return db->EscapeString(account.email);
        if (token.value == "auth_token")
            return db->EscapeString(account.authToken);
        if (token.value == "auth_token_expiry")
            return std::to_string(account.authTokenExpiry);
        if (token.value == "type")
            return std::to_string(static_cast<int>(account.type));
        if (token.value == "status")
            return std::to_string(static_cast<int>(account.status));
        if (token.value == "creation")
            return std::to_string(account.creation);
        if (token.value == "char_slots")
            return std::to_string(account.charSlots);
        if (token.value == "current_character_uuid")
            return db->EscapeString(account.currentCharacterUuid);
        if (token.value == "current_server_uuid")
            return db->EscapeString(account.currentServerUuid);
        if (token.value == "online_status")
            return std::to_string(static_cast<int>(account.onlineStatus));
        if (token.value == "guild_uuid")
            return db->EscapeString(account.guildUuid);
        if (token.value == "chest_size")
            return std::to_string(account.chest_size);
        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
    default:
        return token.value;
    }
}

bool DBAccount::Create(AB::Entities::Account& account)
{
    static constexpr const char* SQL =
        "INSERT INTO accounts ("
            "uuid, name, password, email, type, status, creation, "
            "char_slots, current_server_uuid, online_status, guild_uuid, chest_size "
        ") VALUES ( "
            "${uuid}, ${name}, ${password}, ${email}, ${type}, ${status}, ${creation}, "
            "${char_slots}, ${current_server_uuid}, ${online_status}, ${guild_uuid}, ${chest_size}"
        ")";
    if (Utils::Uuid::IsEmpty(account.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, account, std::placeholders::_1))))
        return false;

    if (!transaction.Commit())
        return false;

    return true;
}

bool DBAccount::Load(AB::Entities::Account& account)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT * FROM accounts WHERE uuid= ${uuid}";
    static constexpr const char* SQL_NAME = "SELECT * FROM accounts WHERE name= ${name}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(account.uuid))
        sql = SQL_UUID;
    else if (!account.name.empty())
        sql = SQL_NAME;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    ASSERT(sql);

    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, account, std::placeholders::_1));
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
    {
        LOG_ERROR << "No record found for " << query << std::endl;
        return false;
    }

    account.uuid = result->GetString("uuid");
    account.name = result->GetString("name");
    account.password = result->GetString("password");
    account.email = result->GetString("email");
    account.authToken = result->GetString("auth_token");
    account.authTokenExpiry = result->GetLong("auth_token_expiry");
    account.type = static_cast<AB::Entities::AccountType>(result->GetInt("type"));
    account.status = static_cast<AB::Entities::AccountStatus>(result->GetInt("status"));
    account.creation = result->GetLong("creation");
    account.charSlots = result->GetUInt("char_slots");
    account.currentCharacterUuid = result->GetString("current_character_uuid");
    account.currentServerUuid = result->GetString("current_server_uuid");
    account.onlineStatus = static_cast<AB::Entities::OnlineStatus>(result->GetInt("online_status"));
    account.guildUuid = result->GetString("guild_uuid");
    account.chest_size = static_cast<uint16_t>(result->GetInt("chest_size"));

    LoadCharacters(account);

    return true;
}

void DBAccount::LoadCharacters(AB::Entities::Account& account)
{
    Database* db = GetSubsystem<Database>();
    account.characterUuids.clear();
    static constexpr const char* SQL = "SELECT uuid, name FROM players WHERE account_uuid = ${account_uuid} ORDER BY name";

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, account, std::placeholders::_1))); result; result = result->Next())
    {
        account.characterUuids.push_back(result->GetString("uuid"));
    }
}

bool DBAccount::Save(const AB::Entities::Account& account)
{
    if (Utils::Uuid::IsEmpty(account.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "UPDATE accounts SET "
        "password = ${password}, "
        "email = ${email}, "
        "auth_token = ${auth_token}, "
        "auth_token_expiry = ${auth_token_expiry}, "
        "type = ${type}, "
        "status = ${status}, "
        "char_slots = ${char_slots}, "
        "current_character_uuid = ${current_character_uuid}, "
        "current_server_uuid = ${current_server_uuid} , "
        "online_status = ${online_status}, "
        "guild_uuid = ${guild_uuid}, "
        "chest_size = ${chest_size} "
        "WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, account, std::placeholders::_1))))
        return false;

    return transaction.Commit();
}

bool DBAccount::Delete(const AB::Entities::Account& account)
{
    if (Utils::Uuid::IsEmpty(account.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "DELETE FROM accounts WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, account, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBAccount::Exists(const AB::Entities::Account& account)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT COUNT(*) AS count FROM accounts WHERE uuid= ${uuid}";
    static constexpr const char* SQL_NAME = "SELECT COUNT(*) AS count FROM accounts WHERE name= ${name}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(account.uuid))
        sql = SQL_UUID;
    else if (!account.name.empty())
        sql = SQL_NAME;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    ASSERT(sql);

    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, account, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

bool DBAccount::LogoutAll()
{
    Database* db = GetSubsystem<Database>();
    static const std::string query = "UPDATE accounts SET online_status = 0";
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

}
