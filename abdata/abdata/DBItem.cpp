#include "stdafx.h"
#include "DBItem.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBItem::Create(AB::Entities::Item& item)
{
    // Do nothing
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBItem::Load(AB::Entities::Item& item)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_items` WHERE ";
    if (!item.uuid.empty() && !uuids::uuid(item.uuid).nil())
        query << "`uuid` = " << db->EscapeString(item.uuid);
    else if (item.index != 0)
        query << "`idx` = " << item.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    item.uuid = result->GetString("uuid");
    item.index = result->GetUInt("idx");
    item.name = result->GetString("name");
    item.script = result->GetString("schript_file");
    item.server_icon = result->GetString("server_icon_file");
    item.server_model = result->GetString("server_model_file");
    item.client_model = result->GetString("client_model_file");
    item.client_icon = result->GetString("client_icon_file");
    item.type = static_cast<AB::Entities::ItemType>(result->GetUInt("type"));
    item.dropChance = static_cast<uint8_t>(result->GetUInt("drop_chance"));

    return true;
}

bool DBItem::Save(const AB::Entities::Item& item)
{
    // Do nothing
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBItem::Delete(const AB::Entities::Item& item)
{
    // Do nothing
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBItem::Exists(const AB::Entities::Item& item)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_items` WHERE ";
    if (!item.uuid.empty() && !uuids::uuid(item.uuid).nil())
        query << "`uuid` = " << db->EscapeString(item.uuid);
    else if (item.index != 0)
        query << "`idx` = " << item.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
