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

bool DBItemPrice::Load(AB::Entities::ItemPrice& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream itemQuery;
    itemQuery << "SELECT `type`, `value` FROM `game_items` WHERE `uuid` = " << db->EscapeString(item.uuid);
    std::shared_ptr<DB::DBResult> itemResult = db->StoreQuery(itemQuery.str());
    if (!itemResult)
        return false;
    AB::Entities::ItemType type = static_cast<AB::Entities::ItemType>(itemResult->GetUInt("type"));

    uint32_t value = itemResult->GetUInt("value");
    uint32_t avail = 0;
    uint32_t minValue = 1;

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

    item.lastCalc = Utils::Tick();
    item.countAvail = avail;
    item.priceBase = value;

    // TODO: For now lets just take the merchant 10%. In the future the price should be calculated according supply and demand.
    float sellFactor = 1.1f;
    float buyFactor = 0.9f;

    item.priceSell = static_cast<int>(item.priceBase * sellFactor);
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
