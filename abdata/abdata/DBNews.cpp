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

#include "DBNews.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::News& v, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(v.uuid);
        if (token.value == "created")
            return std::to_string(v.created);
        if (token.value == "body")
            return db->EscapeString(v.body);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBNews::Create(AB::Entities::News& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "INSERT INTO news ("
        "uuid, created, body"
        ") VALUES ("
        "${uuid}, ${created}, ${body}"
        ")";

    Database* db = GetSubsystem<Database>();

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBNews::Load(AB::Entities::News& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM news WHERE uuid = ${uuid}");

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    v.uuid = result->GetString("uuid");
    v.created = result->GetLong("created");
    v.body = result->GetString("body");

    return true;
}

bool DBNews::Save(const AB::Entities::News& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "UPDATE news SET "
        "created = ${created}, "
        "body = ${body} "
        "WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBNews::Delete(const AB::Entities::News& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "DELETE FROM news WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBNews::Exists(const AB::Entities::News& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) FROM news WHERE uuid = ${uuid}");

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, v, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
