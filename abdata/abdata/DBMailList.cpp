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

bool DBMailList::Save(const AB::Entities::MailList&)
{
    return false;
}

bool DBMailList::Delete(const AB::Entities::MailList&)
{
    return false;
}

bool DBMailList::Exists(const AB::Entities::MailList&)
{
    return false;
}

}
