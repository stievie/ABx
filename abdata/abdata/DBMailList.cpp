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

    std::ostringstream query;
    query << "SELECT `uuid` FROM `mails` WHERE `to_account_uuid` = " << db->EscapeString(ml.uuid);
    // Newest first
    query << " ORDER BY `created` DESC";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ml.mailUuids.clear();
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        ml.mailUuids.push_back(result->GetString("uuid"));
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
