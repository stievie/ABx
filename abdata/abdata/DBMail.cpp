#include "stdafx.h"
#include "DBMail.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

bool DBMail::Create(AB::Entities::Mail& mail)
{
    if (mail.uuid.empty() || uuids::uuid(mail.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `mails` (`uuid`, `from_account_uuid`, `to_account_uuid`, `subject`, `message`, `created`, `is_read`";
    query << ") VALUES (";

    query << db->EscapeString(mail.uuid) << ", ";
    query << db->EscapeString(mail.fromAccountUuid) << ", ";
    query << db->EscapeString(mail.toAccountUuid) << ", ";
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
    if (mail.uuid.empty() || uuids::uuid(mail.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `mails` WHERE `uuid` = " << db->EscapeString(mail.uuid);
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    mail.uuid = result->GetString("uuid");
    mail.fromAccountUuid = result->GetString("from_account_uuid");
    mail.toAccountUuid = result->GetString("to_account_uuid");
    mail.subject = result->GetString("subject");
    mail.message = result->GetString("message");
    mail.created = result->GetLong("created");
    mail.isRead = result->GetUInt("is_read") != 0;
    return true;
}

bool DBMail::Save(const AB::Entities::Mail& mail)
{
    if (mail.uuid.empty() || uuids::uuid(mail.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "UPDATE `mails` SET ";
    query << " `from_account_uuid` = " << db->EscapeString(mail.fromAccountUuid) << ", ";
    query << " `to_account_uuid` = " << db->EscapeString(mail.toAccountUuid) << ", ";
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
    if (mail.uuid.empty() || uuids::uuid(mail.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
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
    if (mail.uuid.empty() || uuids::uuid(mail.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `mails` WHERE `uuid` = " << db->EscapeString(mail.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
