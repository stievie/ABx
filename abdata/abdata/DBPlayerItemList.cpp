#include "stdafx.h"
#include "DBPlayerItemList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBPlayerItemList::Create(AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBPlayerItemList::Load(AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `concrete_items` WHERE `player_uuid` = " << db->EscapeString(il.uuid);
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

bool DBPlayerItemList::Save(const AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBPlayerItemList::Delete(const AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBPlayerItemList::Exists(const AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
