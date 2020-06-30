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

#include "DBMerchantItem.h"
#include <AB/Entities/ConcreteItem.h>

namespace DB {

bool DBMerchantItem::Create(AB::Entities::MerchantItem&)
{
    return true;
}

bool DBMerchantItem::Load(AB::Entities::MerchantItem& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT uuid FROM `concrete_items` WHERE ";
    query << "`deleted` = 0 AND `item_uuid` = " << db->EscapeString(item.uuid) <<
        " AND `storage_place` = " << static_cast<int>(AB::Entities::StoragePlace::Merchant);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    item.concreteUuid = result->GetString("uuid");
    return true;
}

bool DBMerchantItem::Save(const AB::Entities::MerchantItem&)
{
    return true;
}

bool DBMerchantItem::Delete(const AB::Entities::MerchantItem&)
{
    return true;
}

bool DBMerchantItem::Exists(const AB::Entities::MerchantItem& item)
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
