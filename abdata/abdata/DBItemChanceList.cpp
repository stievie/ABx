#include "stdafx.h"
#include "DBItemChanceList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBItemChanceList::Create(AB::Entities::ItemChanceList& il)
{
    if (il.uuid.empty() || uuids::uuid(il.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBItemChanceList::Load(AB::Entities::ItemChanceList& il)
{
    if (il.uuid.empty() || uuids::uuid(il.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `item_uuid`, `chance` FROM `game_item_chances` WHERE `map_uuid` = " << db->EscapeString(il.uuid);
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        il.items.push_back(
            std::make_pair<std::string, float>(
                result->GetString("item_uuid"),
                static_cast<float>(result->GetUInt("chance")) / 1000.0f
            )
        );
    }
    return true;
}

bool DBItemChanceList::Save(const AB::Entities::ItemChanceList& il)
{
    if (il.uuid.empty() || uuids::uuid(il.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBItemChanceList::Delete(const AB::Entities::ItemChanceList& il)
{
    if (il.uuid.empty() || uuids::uuid(il.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBItemChanceList::Exists(const AB::Entities::ItemChanceList& il)
{
    if (il.uuid.empty() || uuids::uuid(il.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
