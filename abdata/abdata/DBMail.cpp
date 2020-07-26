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

namespace DB {

bool DBMail::Create(AB::Entities::Mail& mail)
{
    if (Utils::Uuid::IsEmpty(mail.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    // Check if exceeding mail limit, if yes fail
    query << "SELECT COUNT(*) AS `count` FROM `mails` WHERE `to_account_uuid` = " << db->EscapeString(mail.toAccountUuid);
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    if (result->GetUInt("count") >= AB::Entities::Limits::MAX_MAIL_COUNT)
        return false;

    query.str("");
    query << "INSERT INTO `mails` (`uuid`, `from_account_uuid`, `to_account_uuid`, `from_name`, `to_name`, " <<
        "`subject`, `message`, `created`, `is_read`";
    query << ") VALUES (";

    query << db->EscapeString(mail.uuid) << ", ";
    query << db->EscapeString(mail.fromAccountUuid) << ", ";
    query << db->EscapeString(mail.toAccountUuid) << ", ";
    query << db->EscapeString(mail.fromName) << ", ";
    query << db->EscapeString(mail.toName) << ", ";
    query << db->EscapeString(mail.subject) << ", ";
    query << db->EscapeString(mail.message) << ", ";
    query << mail.created << ", ";
    query << (mail.isRead ? 1 : 0);

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBMail::Load(AB::Entities::Mail& mail)
{
    if (Utils::Uuid::IsEmpty(mail.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `mails` WHERE `uuid` = " << db->EscapeString(mail.uuid);
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
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
    std::ostringstream query;

    query << "UPDATE `mails` SET ";
    query << " `from_account_uuid` = " << db->EscapeString(mail.fromAccountUuid) << ", ";
    query << " `to_account_uuid` = " << db->EscapeString(mail.toAccountUuid) << ", ";
    query << " `from_name` = " << db->EscapeString(mail.fromName) << ", ";
    query << " `to_name` = " << db->EscapeString(mail.toName) << ", ";
    query << " `subject` = " << db->EscapeString(mail.subject) << ", ";
    query << " `message` = " << db->EscapeString(mail.message) << ", ";
    query << " `created` = " << mail.created << ", ";
    query << " `is_read` = " << (mail.isRead ? 1 : 0);

    query << " WHERE `uuid` = " << db->EscapeString(mail.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
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
    std::ostringstream query;
    query << "DELETE FROM `mails` WHERE `uuid` = " << db->EscapeString(mail.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
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

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `mails` WHERE `uuid` = " << db->EscapeString(mail.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
