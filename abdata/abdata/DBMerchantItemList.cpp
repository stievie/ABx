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

#include "DBMerchantItemList.h"
#include <sa/TemplateParser.h>
#include <sa/time.h>
#include <abscommon/Profiler.h>

namespace DB {

bool DBMerchantItemList::Create(AB::Entities::MerchantItemList&)
{
    return true;
}

bool DBMerchantItemList::Load(AB::Entities::MerchantItemList& il)
{
    Database* db = GetSubsystem<Database>();
    // Return a list of items, which are either stackable or were recently sold
    static constexpr const char* SQL = "SELECT concrete_items.uuid AS concrete_uuid, concrete_items.item_uuid AS item_uuid, concrete_items.sold AS sold, "
        "game_items.type AS type, game_items.idx AS idx, game_items.name AS name, game_items.item_flags AS item_flags "
        "FROM concrete_items "
        "LEFT JOIN game_items on game_items.uuid = concrete_items.item_uuid "
        "WHERE deleted = 0 AND storage_place = ${storage_place} "
        "AND ((sold + ${keep_time} > ${current_time}) OR (item_flags & ${stackable_flag} != 0)) "
        "ORDER BY type DESC, name ASC";
    const std::string statement = sa::templ::Parser::Evaluate(SQL, [](const sa::templ::Token& token) -> std::string
    {
        using namespace sa::time::literals;

        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "storage_place")
                return std::to_string(static_cast<int>(AB::Entities::StoragePlace::Merchant));
            if (token.value == "keep_time")
                return std::to_string(1_W);
            if (token.value == "current_time")
                return std::to_string(sa::time::tick());
            if (token.value == "stackable_flag")
                return std::to_string(AB::Entities::ItemFlagStackable);
            LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
            return "";
        default:
            return token.value;
        }
    });

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(statement); result; result = result->Next())
    {
        il.items.push_back({
            result->GetUInt("idx"),
            static_cast<AB::Entities::ItemType>(result->GetUInt("type")),
            result->GetString("name"),
            result->GetString("concrete_uuid"),
            result->GetString("item_uuid")
        });
    }
    return true;
}

bool DBMerchantItemList::Save(const AB::Entities::MerchantItemList&)
{
    return true;
}

bool DBMerchantItemList::Delete(const AB::Entities::MerchantItemList&)
{
    return true;
}

bool DBMerchantItemList::Exists(const AB::Entities::MerchantItemList&)
{
    return true;
}

}
