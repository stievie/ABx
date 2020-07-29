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

#include "DBAccountBan.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::AccountBan& ban, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(ban.uuid);
        if (token.value == "ban_uuid")
            return db->EscapeString(ban.banUuid);
        if (token.value == "account_uuid")
            return db->EscapeString(ban.accountUuid);
        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBAccountBan::Create(AB::Entities::AccountBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    static constexpr const char* SQL = "INSERT INTO account_bans ("
                "uuid, ban_uuid, account_uuid"
            ") VALUES ("
                "${uuid}, ${ban_uuid}, ${account_uuid}"
            ")";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBAccountBan::Load(AB::Entities::AccountBan& ban)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT * FROM account_bans WHERE uuid = ${uuid}";
    static constexpr const char* SQL_ACCOUNT = "SELECT * FROM account_bans WHERE account_uuid = ${account_uuid}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        sql = SQL_UUID;
    else if (!ban.accountUuid.empty())
        sql = SQL_ACCOUNT;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
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

    static constexpr const char* SQL = "UPDATE account_bans SET "
        "ban_uuid = ${ban_uuid}, "
        "account_uuid = ${account_uuid}, "
        "WHERE uuid = ${uuid}";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1))))
        return false;

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
    static constexpr const char* SQL = "DELETE FROM account_bans WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBAccountBan::Exists(const AB::Entities::AccountBan& ban)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT COUNT(*) AS count FROM account_bans WHERE uuid = ${uuid}";
    static constexpr const char* SQL_ACCOUNT = "SELECT COUNT(*) AS count FROM account_bans WHERE account_uuid = ${account_uuid}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        sql = SQL_UUID;
    else if (!ban.accountUuid.empty())
        sql = SQL_ACCOUNT;
    else
    {
        LOG_ERROR << "UUID and Account UUID are empty" << std::endl;
        return false;
    }

    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    return result->GetUInt("count") != 0;
}

}
