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

#include "DBIpBan.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::IpBan& ban, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "ip")
            return std::to_string(ban.ip);
        if (token.value == "mask")
            return std::to_string(ban.mask);
        if (token.value == "uuid")
            return db->EscapeString(ban.uuid);
        if (token.value == "ban_uuid")
            return db->EscapeString(ban.banUuid);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBIpBan::Create(AB::Entities::IpBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL_SELECT = "SELECT COUNT(*) as count FROM ip_bans WHERE "
        "((${ip} & ${mask} & mask) = (ip & mask & ${mask}))";

    Database* db = GetSubsystem<Database>();
    const std::string selectQuery = sa::templ::Parser::Evaluate(SQL_SELECT, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(selectQuery);
    if (result && result->GetInt("count") != 0)
    {
        LOG_ERROR << "There is already a record matching this IP and mask" << std::endl;
        return false;
    }

    static constexpr const char* SQL_INSERT = "INSERT INTO ip_bans ("
            "uuid, ban_uuid, ip, mask"
        ") VALUES ("
            "${uuid}, ${ban_uuid}, ${ip}, ${mask}"
        ")";

    const std::string query = sa::templ::Parser::Evaluate(SQL_INSERT, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBIpBan::Load(AB::Entities::IpBan& ban)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM ip_bans WHERE ");
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (ban.ip != 0)
    {
        if (ban.mask == 0)
        {
            LOG_ERROR << "IP mask is 0 it would match all IPs" << std::endl;
            return false;
        }
        parser.Append("((${ip} & ${mask} & mask) = (ip & mask & ${mask}))", tokens);
    }
    else
    {
        LOG_ERROR << "UUID and IP are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    ban.uuid = result->GetString("uuid");
    ban.banUuid = result->GetString("ban_uuid");
    ban.ip = result->GetUInt("ip");
    ban.mask = result->GetUInt("mask");

    return true;
}

bool DBIpBan::Save(const AB::Entities::IpBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "UPDATE ip_bans SET "
        "ban_uuid = ${ban_uuid}, "
        "ip = ${ip}, "
        "mask = ${mask}, "
        "WHERE uuid = $[uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBIpBan::Delete(const AB::Entities::IpBan& ban)
{
    if (Utils::Uuid::IsEmpty(ban.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "DELETE FROM ip_bans WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBIpBan::Exists(const AB::Entities::IpBan& ban)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) AS count FROM ip_bans WHERE ");
    if (!Utils::Uuid::IsEmpty(ban.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (ban.ip != 0)
    {
        if (ban.mask == 0)
        {
            LOG_ERROR << "IP mask is 0 it would match all IPs" << std::endl;
            return false;
        }
        parser.Append("((${ip} & ${mask} & mask) = (ip & mask & ${mask}))", tokens);
    }
    else
    {
        LOG_ERROR << "UUID and IP are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, ban, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
