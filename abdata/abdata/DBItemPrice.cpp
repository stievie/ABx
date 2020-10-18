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
#include <sa/TemplateParser.h>
#include <sa/time.h>

namespace DB {

bool DBItemPrice::Create(AB::Entities::ItemPrice&)
{
    return true;
}

uint32_t DBItemPrice::GetDropChance(const std::string& itemUuid)
{
    static constexpr const char* SQL = "SELECT AVG(chance) AS avg_chance FROM game_item_chances WHERE item_uuid = ${item_uuid}"
        " GROUP BY item_uuid";

    Database* db = GetSubsystem<Database>();

    const std::string query = sa::templ::Parser::Evaluate(SQL, [db, &itemUuid](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "item_uuid")
                return db->EscapeString(itemUuid);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return 0;
    return result->GetUInt("avg_chance");
}

uint32_t DBItemPrice::GetAvgValue(const std::string& itemUuid)
{
    static constexpr const char* SQL = "SELECT AVG(value) as avg_value FROM concrete_items WHERE deleted = 0 "
        "AND item_uuid = ${item_uuid} GROUP BY item_uuid";

    Database* db = GetSubsystem<Database>();
    const std::string query = sa::templ::Parser::Evaluate(SQL, [db, &itemUuid](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "item_uuid")
                return db->EscapeString(itemUuid);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (result)
        return result->GetUInt("avg_value");
    return 0;
}

uint32_t DBItemPrice::GetAvailable(const std::string& itemUuid)
{
    // How many of that belongs to the merchant. Since sold non-stackable items are no longer available after some time,
    // we must filter them out.
    static constexpr const char* SQL = "SELECT SUM(count) AS available FROM concrete_items "
        "LEFT JOIN game_items on game_items.uuid = concrete_items.item_uuid "
        "WHERE deleted = 0 AND storage_place = ${storage_place} "
        "AND ((sold + ${keep_time} > ${current_time}) OR (item_flags & ${stackable_flag} != 0)) "
        "AND item_uuid = ${item_uuid} "
        "GROUP BY item_uuid";

    Database* db = GetSubsystem<Database>();
    const std::string query = sa::templ::Parser::Evaluate(SQL, [db, &itemUuid](const sa::templ::Token& token) -> std::string
    {
        using namespace sa::time::literals;
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "item_uuid")
                return db->EscapeString(itemUuid);
            if (token.value == "storage_place")
                return std::to_string(static_cast<int>(AB::Entities::StoragePlace::Merchant));
            if (token.value == "keep_time")
                return std::to_string(1_W);
            if (token.value == "current_time")
                return std::to_string(sa::time::tick());
            if (token.value == "stackable_flag")
                return std::to_string(AB::Entities::ItemFlagStackable);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (result)
        return result->GetUInt("available");
    return 0;

}

bool DBItemPrice::Load(AB::Entities::ItemPrice& item)
{
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT type, item_flags, value FROM game_items WHERE uuid = ${uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, [db, &item](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "uuid")
                return db->EscapeString(item.uuid);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    std::shared_ptr<DB::DBResult> itemResult = db->StoreQuery(query);
    if (!itemResult)
        return false;
    AB::Entities::ItemType type = static_cast<AB::Entities::ItemType>(itemResult->GetUInt("type"));
    uint32_t itemFlags = itemResult->GetUInt("item_flags");

    uint32_t value = itemResult->GetUInt("value");
    uint32_t avail = 0;
    uint32_t minValue = 1;

    uint32_t dropChance = GetDropChance(item.uuid);

    if (type != AB::Entities::ItemType::Material)
        value = std::max(value, GetAvgValue(item.uuid));
    else
        minValue = value;

    avail = std::max(avail, GetAvailable(item.uuid));

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

    static constexpr const char* SQL = "SELECT COUNT(*) AS count FROM concrete_items WHERE "
        "deleted = 0 AND item_uuid = ${item_uuid} AND storage_place = ${storage_place}";
    Database* db = GetSubsystem<Database>();

    const std::string query = sa::templ::Parser::Evaluate(SQL, [db, &item](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "item_uuid")
                return db->EscapeString(item.uuid);
            if (token.value == "storage_place")
                return std::to_string(static_cast<int>(AB::Entities::StoragePlace::Merchant));
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
