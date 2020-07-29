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

#include "DBReservedName.h"
#include "StorageProvider.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::ReservedName& rn, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(rn.uuid);
        if (token.value == "name")
            return db->EscapeString(rn.name);
        if (token.value == "is_reserved")
            return std::to_string(rn.isReserved ? 1 : 0);
        if (token.value == "reserved_for_account_uuid")
            return db->EscapeString(rn.reservedForAccountUuid);
        if (token.value == "expires")
            return std::to_string(rn.expires);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

// Player names are case insensitive. The DB needs a proper index for that:
// CREATE INDEX reserved_names_name_ci_index ON reserved_names USING btree (lower(name))

bool DBReservedName::Create(AB::Entities::ReservedName& rn)
{
    if (Utils::Uuid::IsEmpty(rn.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "INSERT INTO reserved_names ("
            "uuid, name, is_reserved, reserved_for_account_uuid, expires"
        ") VALUES ("
            "${uuid}, ${name}, ${is_reserved}, ${reserved_for_account_uuid}, ${expires}"
        ")";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, rn, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBReservedName::Load(AB::Entities::ReservedName& n)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM reserved_names WHERE ");
    if (!Utils::Uuid::IsEmpty(n.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (!n.name.empty())
        parser.Append("LOWER(name) = LOWER(${name})", tokens);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, n, std::placeholders::_1));
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    n.uuid = result->GetString("uuid");
    n.name = result->GetString("name");
    n.isReserved = result->GetInt("is_reserved") != 0;
    n.reservedForAccountUuid = result->GetString("reserved_for_account_uuid");
    n.expires = result->GetLong("expires");

    return true;
}

bool DBReservedName::Save(const AB::Entities::ReservedName& rn)
{
    if (Utils::Uuid::IsEmpty(rn.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    // Only these may be changed
    static constexpr const char* SQL = "UPDATE reserved_names SET "
        "is_reserved = ${is_reserved}, "
        "expires = ${expires} "
        "WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, rn, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBReservedName::Delete(const AB::Entities::ReservedName& rn)
{
    if (Utils::Uuid::IsEmpty(rn.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "DELETE FROM reserved_names WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, rn, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBReservedName::Exists(const AB::Entities::ReservedName& n)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) AS count FROM reserved_names WHERE ");
    if (!Utils::Uuid::IsEmpty(n.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (!n.name.empty())
        parser.Append("LOWER(name) = LOWER(${name})", tokens);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, n, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

void DBReservedName::DeleteExpired(StorageProvider* sp)
{
    // When expires == 0 it does not expire, otherwise it's the time stamp
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT uuid FROM reserved_names WHERE (expires <> 0 AND expires < ${expires})";
    AB::Entities::ReservedName n;
    n.expires = Utils::Tick();
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, n, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        // No matches
        return;

    std::vector<std::string> rns;
    for (; result; result = result->Next())
    {
        rns.push_back(result->GetString("uuid"));
    }

    // First invalidate them
    for (const std::string& g : rns)
    {
        AB::Entities::ReservedName rn;
        rn.uuid = g;
        sp->EntityInvalidate(rn);
    }

    // Then delete from DB
    static constexpr const char* SQL_DELETE =  "DELETE FROM reserved_names WHERE (expires <> 0 AND expires < ${expires})";
    const std::string deleteQuery = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, n, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return;

    if (!db->ExecuteQuery(deleteQuery))
        return;

    transaction.Commit();
}

}
