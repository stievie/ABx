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

#include "DBMail.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Mail& mail, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(mail.uuid);
        if (token.value == "from_account_uuid")
            return db->EscapeString(mail.fromAccountUuid);
        if (token.value == "to_account_uuid")
            return db->EscapeString(mail.toAccountUuid);
        if (token.value == "from_name")
            return db->EscapeString(mail.fromName);
        if (token.value == "to_name")
            return db->EscapeString(mail.toName);
        if (token.value == "subject")
            return db->EscapeString(mail.subject);
        if (token.value == "message")
            return db->EscapeString(mail.message);
        if (token.value == "created")
            return std::to_string(mail.created);
        if (token.value == "is_read")
            return std::to_string((mail.isRead ? 1 : 0));

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

uint32_t DBMail::GetMailCount(AB::Entities::Mail& mail)
{
    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM mails WHERE to_account_uuid = ${to_account_uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, mail, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        // Something is wrong!
        return std::numeric_limits<uint32_t>::max();
    return result->GetUInt("count");
}

bool DBMail::Create(AB::Entities::Mail& mail)
{
    if (Utils::Uuid::IsEmpty(mail.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    // Check if exceeding mail limit, if yes fail
    if (GetMailCount(mail) >= AB::Entities::Limits::MAX_MAIL_COUNT)
        return false;

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "INSERT INTO mails ("
        "uuid, from_account_uuid, to_account_uuid, from_name, to_name, subject, message, created, is_read"
        ") VALUES ("
        "${uuid}, ${from_account_uuid}, ${to_account_uuid}, ${from_name}, ${to_name}, ${subject}, ${message}, ${created}, ${is_read}"
        ")";

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, mail, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;
    if (!db->ExecuteQuery(query))
        return false;

    return  transaction.Commit();
}

bool DBMail::Load(AB::Entities::Mail& mail)
{
    if (Utils::Uuid::IsEmpty(mail.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT * FROM mails WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, mail, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    mail.uuid = result->GetString("uuid");
    mail.fromAccountUuid = result->GetString("from_account_uuid");
    mail.toAccountUuid = result->GetString("to_account_uuid");
    mail.fromName = result->GetString("from_name");
    mail.toName = result->GetString("to_name");
    mail.subject = result->GetString("subject");
    mail.message = result->GetString("message");
    mail.created = result->GetLong("created");
    mail.isRead = result->GetUInt("is_read") != 0;
    return true;
}

bool DBMail::Save(const AB::Entities::Mail& mail)
{
    if (Utils::Uuid::IsEmpty(mail.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "UPDATE mails SET "
        "from_account_uuid = ${from_account_uuid}, "
        "to_account_uuid = ${to_account_uuid}, "
        "from_name = ${from_name}, "
        "to_name = ${to_name}, "
        "subject = ${subject}, "
        "message = ${message}, "
        "created = ${created}, "
        "is_read = ${is_read} "
        "WHERE uuid = ${uuid}";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, mail, std::placeholders::_1))))
        return false;

    return transaction.Commit();
}

bool DBMail::Delete(const AB::Entities::Mail& mail)
{
    if (Utils::Uuid::IsEmpty(mail.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "DELETE FROM mails WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, mail, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;
    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBMail::Exists(const AB::Entities::Mail& mail)
{
    if (Utils::Uuid::IsEmpty(mail.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM mails WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, mail, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
