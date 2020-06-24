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

    std::ostringstream query;
    query << "SELECT concrete_items.*, game_items.type FROM `concrete_items` ";
    query << "LEFT JOIN game_items ON concrete_items.item_uuid = game_items.uuid ";
    query << "WHERE `deleted` = 0 AND `item_uuid` = " << db->EscapeString(item.uuid) <<
        " AND `storage_place` = " << static_cast<int>(AB::Entities::StoragePlace::Merchant);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    item.lastCalc = Utils::Tick();
    item.countAvail = result->GetUInt("count");
    item.priceBase = result->GetUInt("value");
    // TODO: For now lets just take the merchant 10%. In the future the price should be calculated according supply and demand.
    item.priceSell = static_cast<int>(((float)item.priceBase / 100.0f) * 110.0f);
    item.priceBuy = static_cast<int>(((float)item.priceBase / 100.0f) * 90.0f);

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
