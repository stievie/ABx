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

#include "DBAccountKey.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::AccountKey& ak, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(ak.uuid);
        if (token.value == "used")
            return std::to_string(ak.used);
        if (token.value == "total")
            return std::to_string(ak.total);
        if (token.value == "description")
            return db->EscapeString(ak.description);
        if (token.value == "status")
            return std::to_string(static_cast<int>(ak.status));
        if (token.value == "key_type")
            return std::to_string(static_cast<int>(ak.type));
        if (token.value == "email")
            return db->EscapeString(ak.email);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBAccountKey::Create(AB::Entities::AccountKey& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "INSERT INTO account_keys ("
            "uuid, used, total, description, status, key_type, email"
        ") VALUES ( "
            "${uuid}, ${used}, ${total}, ${description}, ${status}, ${key_type}, ${email}"
        ")";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ak, std::placeholders::_1));
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBAccountKey::Load(AB::Entities::AccountKey& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM account_keys WHERE uuid = ${uuid}");
    if (ak.status != AB::Entities::AccountKeyStatus::KeyStatusUnknown)
        parser.Append(" AND status = ${status}", tokens);
    if (ak.type != AB::Entities::AccountKeyType::KeyTypeUnknown)
        parser.Append(" AND key_type = ${key_type}", tokens);

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, ak, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    ak.uuid = result->GetString("uuid");
    ak.used = static_cast<uint16_t>(result->GetUInt("used"));
    ak.total = static_cast<uint16_t>(result->GetUInt("total"));
    ak.description = result->GetString("description");
    ak.status = static_cast<AB::Entities::AccountKeyStatus>(result->GetUInt("status"));
    ak.type = static_cast<AB::Entities::AccountKeyType>(result->GetUInt("key_type"));
    ak.email = result->GetString("email");
    return true;
}

bool DBAccountKey::Save(const AB::Entities::AccountKey& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "UPDATE account_keys SET "
        "used = ${used}, "
        "total = ${total}, "
        "description = ${description}, "
        "status = ${status}, "
        "key_type = ${key_type}, "
        "email = ${email} "
        "WHERE uuid = ${uuid}";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, ak, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBAccountKey::Delete(const AB::Entities::AccountKey&)
{
    // Can not delete
    return true;
}

bool DBAccountKey::Exists(const AB::Entities::AccountKey& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) AS count FROM account_keys WHERE uuid = ${uuid}");
    if (ak.status != AB::Entities::AccountKeyStatus::KeyStatusUnknown)
        parser.Append(" AND status = ${status}", tokens);
    if (ak.type != AB::Entities::AccountKeyType::KeyTypeUnknown)
        parser.Append(" AND key_type = ${key_type}", tokens);

    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, ak, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
