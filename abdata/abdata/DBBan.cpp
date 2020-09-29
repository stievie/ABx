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

#include "DBBan.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Ban& ban, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(ban.uuid);
        if (token.value == "expires")
            return std::to_string(ban.expires);
        if (token.value == "added")
            return std::to_string(ban.added);
        if (token.value == "reason")
            return std::to_string(static_cast<int>(ban.reason));
        if (token.value == "active")
            return std::to_string(ban.active ? 1 : 0);
        if (token.value == "admin_uuid")
            return db->EscapeString(ban.adminUuid);
        if (token.value == "comment")
            return db->EscapeString(ban.comment);
        if (token.value == "hits")
            return std::to_string(ban.hits);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBBan::Create(AB::Entities::Ban& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "INSERT INTO bans ("
            "uuid, expires, added, reason, active, admin_uuid, comment, hits"
        ") VALUES ("
            "${uuid}, ${expires}, ${added}, ${reason}, ${active}, ${admin_uuid}, ${comment}, ${hits}"
        ")";

    Database* db = GetSubsystem<Database>();

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBBan::Load(AB::Entities::Ban& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT * FROM bans WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    ban.uuid = result->GetString("uuid");
    ban.expires = result->GetLong("expires");
    ban.added = result->GetLong("added");
    ban.reason = static_cast<AB::Entities::BanReason>(result->GetInt("reason"));
    ban.active = result->GetUInt("active") != 0;
    ban.adminUuid = result->GetString("admin_uuid");
    ban.comment = result->GetString("comment");
    ban.hits = result->GetUInt("hits");

    return true;
}

bool DBBan::Save(const AB::Entities::Ban& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "UPDATE bans SET "
        "expires = ${expires}, "
        "added = ${added}, "
        "reason = ${reason}, "
        "active = ${active}, "
        "admin_uuid = ${admin_uuid}, "
        "comment = ${comment}, "
        "hits = %{hits} "
        "WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBBan::Delete(const AB::Entities::Ban& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "DELETE FROM bans WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBBan::Exists(const AB::Entities::Ban& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM bans WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
