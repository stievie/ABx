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

#include "DBConcreteItem.h"
#include "StorageProvider.h"
#include <AB/Entities/GameInstance.h>
#include <abscommon/Utils.h>
#include <sa/TemplateParser.h>
#include <sa/time.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::ConcreteItem& item, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(item.uuid);
        if (token.value == "player_uuid")
            return db->EscapeString(item.playerUuid);
        if (token.value == "storage_place")
            return std::to_string(static_cast<int>(item.storagePlace));
        if (token.value == "storage_pos")
            return std::to_string(item.storagePos);
        if (token.value == "upgrade_1")
            return db->EscapeString(item.upgrade1Uuid);
        if (token.value == "upgrade_2")
            return db->EscapeString(item.upgrade2Uuid);
        if (token.value == "upgrade_3")
            return db->EscapeString(item.upgrade3Uuid);
        if (token.value == "account_uuid")
            return db->EscapeString(item.accountUuid);
        if (token.value == "item_uuid")
            return db->EscapeString(item.itemUuid);
        if (token.value == "stats")
            return db->EscapeBlob(item.itemStats.data(), item.itemStats.length());
        if (token.value == "count")
            return std::to_string(item.count);
        if (token.value == "creation")
            return std::to_string(item.creation);
        if (token.value == "deleted")
            return std::to_string(item.deleted);
        if (token.value == "value")
            return std::to_string(item.value);
        if (token.value == "instance_uuid")
            return db->EscapeString(item.instanceUuid);
        if (token.value == "map_uuid")
            return db->EscapeString(item.mapUuid);
        if (token.value == "flags")
            return std::to_string(item.flags);
        if (token.value == "sold")
            return std::to_string(item.sold);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBConcreteItem::Create(AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "INSERT INTO concrete_items ("
            "uuid, player_uuid, storage_place, storage_pos, upgrade_1, upgrade_2, upgrade_3, "
            "account_uuid, item_uuid, stats, count, creation, deleted, value, instance_uuid, "
            "map_uuid, flags, sold"
        ") VALUES ("
            "${uuid}, ${player_uuid}, ${storage_place}, ${storage_pos}, ${upgrade_1}, ${upgrade_2}, ${upgrade_3}, "
            "${account_uuid}, ${item_uuid}, ${stats}, ${count}, ${creation}, ${deleted}, ${value}, ${instance_uuid}, "
            "${map_uuid}, ${flags}, ${sold}"
        ")";

    Database* db = GetSubsystem<Database>();

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query))
        return false;

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

    static constexpr const char* SQL = "SELECT * FROM concrete_items WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
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
    item.count = static_cast<uint32_t>(result->GetUInt("count"));
    item.creation = result->GetLong("creation");
    item.deleted = result->GetLong("deleted");
    item.value = static_cast<uint16_t>(result->GetUInt("value"));
    item.instanceUuid = result->GetString("instance_uuid");
    item.mapUuid = result->GetString("map_uuid");
    item.flags = static_cast<uint32_t>(result->GetUInt("flags"));
    item.sold = result->GetLong("sold");

    return true;
}

bool DBConcreteItem::Save(const AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "UPDATE concrete_items SET "
        "player_uuid = ${player_uuid}, "
        "storage_place = ${storage_place}, "
        "storage_pos = ${storage_pos}, "
        "upgrade_1 = ${upgrade_1}, "
        "upgrade_2 = ${upgrade_2}, "
        "upgrade_3 = ${upgrade_3}, "
        "account_uuid = ${account_uuid}, "
        "item_uuid = ${item_uuid}, "
        "stats = ${stats}, "
        "count = ${count}, "
        "creation = ${creation}, "
        "deleted = ${deleted}, "
        "value = ${value}, "
        "instance_uuid = ${instance_uuid}, "
        "map_uuid = ${map_uuid}, "
        "flags = ${flags}, "
        "sold = ${sold} "
        "WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));
    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBConcreteItem::Delete(const AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "DELETE FROM concrete_items WHERE uuid = ${uuid}";

    Database* db = GetSubsystem<Database>();
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    if (!db->ExecuteQuery(query))
        return false;

    return transaction.Commit();
}

bool DBConcreteItem::Exists(const AB::Entities::ConcreteItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM concrete_items WHERE uuid = ${uuid}"
        " AND deleted = 0";

    Database* db = GetSubsystem<Database>();

    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

void DBConcreteItem::Clean(StorageProvider* sp)
{
    LOG_INFO << "Cleaning concrete items" << std::endl;
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT uuid, instance_uuid FROM concrete_items WHERE "
        "storage_place = ${storage_place} "
        "AND deleted = 0";
    AB::Entities::ConcreteItem dummyItem;
    dummyItem.storagePlace = AB::Entities::StoragePlace::Scene;
    const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, dummyItem, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return;

    std::vector<std::string> uuids;
    for (result = db->StoreQuery(query); result; result = result->Next())
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
            if (item.storagePlace == AB::Entities::StoragePlace::Scene)
            {
                item.deleted = sa::time::tick();
                sp->EntityUpdate(item);
                sp->EntityInvalidate(item);
            }
        }
    }
}

}
