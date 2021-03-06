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

#include "DBTypedItemList.h"
#include <abscommon/Profiler.h>
#include <sa/TemplateParser.h>

namespace DB {

bool DBTypedItemList::Create(AB::Entities::TypedItemList&)
{
    return true;
}

bool DBTypedItemList::Load(AB::Entities::TypedItemList& il)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT game_item_chances.chance AS chance, game_items.type AS type, game_items.belongs_to AS belongs_to, game_items.uuid AS uuid, "
        "game_item_chances.map_uuid AS map_uuid "
        "FROM game_item_chances LEFT JOIN game_items ON game_items.uuid = game_item_chances.item_uuid "
        "WHERE (map_uuid = ${map_uuid}");
    if (!Utils::Uuid::IsEmpty(il.uuid))
        parser.Append(" OR map_uuid = ${empty_map_uuid}", tokens);
    parser.Append(")", tokens);
    if (il.type != AB::Entities::ItemType::Unknown)
        parser.Append(" AND type = ${type}", tokens);

    auto callback = [db, &il](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "map_uuid")
                return db->EscapeString(il.uuid);
            if (token.value == "empty_map_uuid")
                return db->EscapeString(Utils::Uuid::EMPTY_UUID);
            if (token.value == "type")
                return std::to_string(static_cast<int>(il.type));
            ASSERT_FALSE();
        default:
            return token.value;
        }
    };
    const std::string query = tokens.ToString(callback);

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
    {
        AB::Entities::TypedListItem c;
        c.uuid = result->GetString("uuid");
        c.belongsTo = static_cast<AB::Entities::ItemType>(result->GetUInt("belongs_to"));
        c.chance = static_cast<float>(result->GetUInt("chance")) / 1000.0f;
        il.items.push_back(c);
    }
    return true;
}

bool DBTypedItemList::Save(const AB::Entities::TypedItemList&)
{
    return true;
}

bool DBTypedItemList::Delete(const AB::Entities::TypedItemList&)
{
    return true;
}

bool DBTypedItemList::Exists(const AB::Entities::TypedItemList&)
{
    return true;
}

}
