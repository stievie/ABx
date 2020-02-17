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

#include "stdafx.h"
#include "DBConcreteItem.h"
#include "StorageProvider.h"
#include <AB/Entities/GameInstance.h>

namespace DB {

bool DBConcreteItem::Create(AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `concrete_items` (`uuid`, `player_uuid`, `storage_place`, `storage_pos`, `upgrade_1`, `upgrade_2`, `upgrade_3`, " <<
        "`account_uuid`, `item_uuid`, `stats`, `count`, `creation`, `value`, `instance_uuid`, `map_uuid`";
    query << ") VALUES (";

    query << db->EscapeString(item.uuid) << ", ";
    query << db->EscapeString(item.playerUuid) << ", ";
    query << static_cast<int>(item.storagePlace) << ", ";
    query << static_cast<int>(item.storagePos) << ", ";
    query << db->EscapeString(item.upgrade1Uuid) << ", ";
    query << db->EscapeString(item.upgrade2Uuid) << ", ";
    query << db->EscapeString(item.upgrade3Uuid) << ", ";
    query << db->EscapeString(item.accountUuid) << ", ";
    query << db->EscapeString(item.itemUuid) << ", ";
    query << db->EscapeBlob(item.itemStats.data(), item.itemStats.length()) << ", ";
    query << static_cast<int>(item.count) << ", ";
    query << item.creation << ", ";
    query << static_cast<int>(item.value) << ", ";
    query << db->EscapeString(item.instanceUuid) << ", ";
    query << db->EscapeString(item.mapUuid);

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

bool DBConcreteItem::Load(AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `concrete_items` WHERE ";
    query << "`uuid` = " << db->EscapeString(item.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    item.uuid = result->GetString("uuid");
    item.playerUuid = result->GetString("player_uuid");
    item.storagePlace = static_cast<AB::Entities::StoragePlace>(result->GetUInt("storage_place"));
    item.storagePos = static_cast<uint16_t>(result->GetUInt("storage_pos"));
    item.upgrade1Uuid = result->GetString("upgrade_1");
    item.upgrade2Uuid = result->GetString("upgrade_2");
    item.upgrade3Uuid = result->GetString("upgrade_3");
    item.accountUuid = result->GetString("account_uuid");
    item.itemUuid = result->GetString("item_uuid");
    item.itemStats = result->GetStream("stats");
    item.count = static_cast<uint16_t>(result->GetUInt("count"));
    item.creation = result->GetLong("creation");
    item.value = static_cast<uint16_t>(result->GetUInt("value"));
    item.instanceUuid = result->GetString("instance_uuid");
    item.mapUuid = result->GetString("map_uuid");

    return true;
}

bool DBConcreteItem::Save(const AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `concrete_items` SET ";

    // Only these may be changed
    query << " `player_uuid` = " << db->EscapeString(item.playerUuid) << ", ";
    query << " `storage_place` = " << static_cast<int>(item.storagePlace) << ", ";
    query << " `storage_pos` = " << static_cast<int>(item.storagePos) << ", ";
    query << " `upgrade_1` = " << db->EscapeString(item.upgrade1Uuid) << ", ";
    query << " `upgrade_2` = " << db->EscapeString(item.upgrade2Uuid) << ", ";
    query << " `upgrade_3` = " << db->EscapeString(item.upgrade3Uuid) << ", ";
    query << " `account_uuid` = " << db->EscapeString(item.accountUuid) << ", ";
    query << " `item_uuid` = " << db->EscapeString(item.itemUuid) << ", ";
    query << " `stats` = " << db->EscapeBlob(item.itemStats.data(), item.itemStats.length()) << ", ";
    query << " `count` = " << static_cast<int>(item.count) << ", ";
    query << " `creation` = " << item.creation << ", ";
    query << " `value` = " << static_cast<int>(item.value) << ", ";
    query << " `instance_uuid` = " << db->EscapeString(item.instanceUuid) << ", ";
    query << " `map_uuid` = " << db->EscapeString(item.mapUuid);

    query << " WHERE `uuid` = " << db->EscapeString(item.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBConcreteItem::Delete(const AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `concrete_items` WHERE `uuid` = " << db->EscapeString(item.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBConcreteItem::Exists(const AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `concrete_items` WHERE ";
    query << "`uuid` = " << db->EscapeString(item.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

void DBConcreteItem::Clean(StorageProvider* sp)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid`, `instance_uuid` FROM `concrete_items` WHERE storage_place = " << static_cast<int>(AB::Entities::StoragePlaceScene);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return;

    std::vector<std::string> uuids;
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        AB::Entities::GameInstance instance;
        instance.uuid = result->GetString("instance_uuid");
        if (sp->EntityRead(instance) && instance.running)
            // Still running
            continue;

        // Not running or not existent, either way, delete it
        uuids.push_back(result->GetString("uuid"));
    }

    for (const std::string& i : uuids)
    {
        AB::Entities::ConcreteItem item;
        item.uuid = i;
        if (!sp->EntityRead(item))
        {
            // Still must check if the item is really in the scene, because
            // DB and cache may have different data.
            if (item.storagePlace == AB::Entities::StoragePlaceScene)
                sp->EntityDelete(item);
        }
    }

}

}
