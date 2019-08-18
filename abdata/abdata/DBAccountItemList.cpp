#include "stdafx.h"
#include "DBAccountItemList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBAccountItemList::Create(AB::Entities::AccountItemList& li)
{
    if (Utils::Uuid::IsEmpty(li.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Load(AB::Entities::AccountItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `concrete_items` WHERE `account_uuid` = " << db->EscapeString(il.uuid);
    if (il.storagePlace != AB::Entities::StoragePlaceNone)
    {
        query << " AND `storage_place` = " << static_cast<int>(il.storagePlace);
    }
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        il.itemUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBAccountItemList::Save(const AB::Entities::AccountItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Delete(const AB::Entities::AccountItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Exists(const AB::Entities::AccountItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
