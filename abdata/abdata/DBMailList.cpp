#include "stdafx.h"
#include "DBMailList.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

bool DBMailList::Create(AB::Entities::MailList&)
{
    return false;
}

bool DBMailList::Load(AB::Entities::MailList& ml)
{
    if (ml.uuid.empty() || uuids::uuid(ml.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    ml.mails.clear();
    std::ostringstream query;
    query << "SELECT `uuid`, `from_name`, `subject`, `created`, `is_read` FROM `mails` WHERE `to_account_uuid` = " << db->EscapeString(ml.uuid);
    // Oldest first because the chat window scrolls down
    query << " ORDER BY `created` ASC";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        // Maybe no mails
        return true;

    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        ml.mails.push_back({
            result->GetString("uuid"),
            result->GetString("from_name"),
            result->GetString("subject"),
            result->GetLong("created"),
            result->GetUInt("is_read") != 0
        });
    }

    return true;
}

bool DBMailList::Save(const AB::Entities::MailList& ml)
{
    if (ml.uuid.empty() || uuids::uuid(ml.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    for (const auto& mail : ml.mails)
    {
        query.str("");
        query << "UPDATE `mails` SET ";
        query << " `is_read` = " << (mail.isRead ? 1 : 0);
        query << " WHERE `uuid` = " << db->EscapeString(mail.uuid);
    }

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBMailList::Delete(const AB::Entities::MailList& ml)
{
    // Delete all mails for this account
    if (ml.uuid.empty() || uuids::uuid(ml.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `mails` WHERE `to_account_uuid` = " << db->EscapeString(ml.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBMailList::Exists(const AB::Entities::MailList&)
{
    return false;
}

}
