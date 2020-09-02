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

#include "DBGuildMembers.h"
#include "StorageProvider.h"
#include <sa/TemplateParser.h>
#include <sa/time.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::GuildMembers& g, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "guild_uuid")
            return db->EscapeString(g.uuid);
        if (token.value == "expires")
            return std::to_string(sa::time::tick());

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBGuildMembers::Create(AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Load(AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT * FROM guild_members WHERE "
        "guild_uuid = ${guild_uuid} "
        "AND (expires = 0 OR expires > ${expires})";

    g.members.clear();
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, g, std::placeholders::_1));

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
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
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Delete(const AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBGuildMembers::Exists(const AB::Entities::GuildMembers& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

void DBGuildMembers::DeleteExpired(StorageProvider* sp)
{
    Database* db = GetSubsystem<Database>();

    auto expires = sa::time::tick();
    auto callback = [expires](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "expires")
                return std::to_string(expires);

            LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
            return "";
        default:
            return token.value;
        }
    };
    static constexpr const char* SQL_SELECT = "SELECT guild_uuid FROM guild_members WHERE "
        "(expires <> 0 AND expires < ${expires})";
    const std::string selectQuery = sa::templ::Parser::Evaluate(SQL_SELECT, callback);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(selectQuery);
    if (!result)
        // No members
        return;

    std::vector<std::string> guilds;
    for (; result; result = result->Next())
    {
        guilds.push_back(result->GetString("guild_uuid"));
    }

    static constexpr const char* SQL_DELETE = "DELETE FROM guild_members WHERE "
        "(expires <> 0 AND expires < ${expires})";
    const std::string deleteQuery = sa::templ::Parser::Evaluate(SQL_SELECT, callback);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return;

    if (!db->ExecuteQuery(deleteQuery))
        return;

    // End transaction
    if (!transaction.Commit())
        return;

    for (const std::string& g : guilds)
    {
        AB::Entities::GuildMembers gms;
        gms.uuid = g;
        sp->EntityInvalidate(gms);
    }
}

}
