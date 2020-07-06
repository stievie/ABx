/**
 * Copyright 2020 Stefan Ascher
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

#include "DBItemPrice.h"
#include <AB/Entities/ConcreteItem.h>
#include <AB/Entities/Item.h>

namespace DB {

bool DBItemPrice::Create(AB::Entities::ItemPrice&)
{
    return true;
}

uint32_t DBItemPrice::GetDropChance(const std::string& itemUuid)
{
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "SELECT AVG(chance) AS avg_chance FROM `game_item_chances` WHERE `item_uuid` = " << db->EscapeString(itemUuid) <<
        " GROUP BY `item_uuid`";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return 0;
    return result->GetUInt("avg_chance");
}

bool DBItemPrice::Load(AB::Entities::ItemPrice& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream itemQuery;
    itemQuery << "SELECT `type`, `item_flags`, `value` FROM `game_items` WHERE `uuid` = " << db->EscapeString(item.uuid);
    std::shared_ptr<DB::DBResult> itemResult = db->StoreQuery(itemQuery.str());
    if (!itemResult)
        return false;
    AB::Entities::ItemType type = static_cast<AB::Entities::ItemType>(itemResult->GetUInt("type"));
    uint32_t itemFlags = itemResult->GetUInt("item_flags");

    uint32_t value = itemResult->GetUInt("value");
    uint32_t avail = 0;
    uint32_t minValue = 1;

    uint32_t dropChance = GetDropChance(item.uuid);

    if (type != AB::Entities::ItemType::Material)
    {
        std::ostringstream query;
        query << "SELECT AVG(value) as avg_value FROM `concrete_items` WHERE deleted = 0 " <<
            " AND item_uuid = " <<
            db->EscapeString(item.uuid) << " GROUP BY item_uuid";
        std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
        if (result)
            value = result->GetUInt("avg_value");
    }
    else
        minValue = value;

    // How many of that belongs to the merchant
    std::ostringstream query;
    query << "SELECT SUM(count) AS available FROM `concrete_items` " <<
        "WHERE `deleted` = 0 AND storage_place = " << static_cast<int>(AB::Entities::StoragePlace::Merchant) <<
        " AND `item_uuid` = " << db->EscapeString(item.uuid) <<
        " GROUP BY `item_uuid`";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (result)
        avail = result->GetUInt("available");

    item.countAvail = avail;
    item.priceBase = value;

    const uint32_t aimCount = AB::Entities::IsItemStackable(itemFlags) ? 250 : 1;
    float sellFactor = 1.25f;
    float buyFactor = 0.85f;
    if (avail > aimCount)
    {
        sellFactor += 9.0f / (float)avail;
        buyFactor += 5.5f / (float)avail;
    }
    else
    {
        sellFactor += 9.0f;
        buyFactor += 5.5f;
    }
    if (dropChance != 0)
    {
        // Although the chance is in promille, we devide by 100 because the chance should't have
        // such a great impact on the price.
        sellFactor /= ((float)dropChance / 100.0f);
        buyFactor /= ((float)dropChance / 100.0f);
    }

    item.priceSell = std::max(static_cast<uint32_t>((float)minValue * 1.5f), static_cast<uint32_t>(item.priceBase * sellFactor));
    item.priceBuy = std::max(minValue, static_cast<uint32_t>(item.priceBase * buyFactor));

    return true;
}

bool DBItemPrice::Save(const AB::Entities::ItemPrice&)
{
    return true;
}

bool DBItemPrice::Delete(const AB::Entities::ItemPrice&)
{
    return true;
}

bool DBItemPrice::Exists(const AB::Entities::ItemPrice& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` `concrete_items` WHERE ";
    query << "`deleted` = 0 AND `item_uuid` = " << db->EscapeString(item.uuid) << " AND `storage_place` = " << static_cast<int>(AB::Entities::StoragePlace::Merchant);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
