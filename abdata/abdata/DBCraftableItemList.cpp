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


#include "DBCraftableItemList.h"
#include <AB/Entities/Item.h>

namespace DB {

bool DBCraftableItemList::Create(AB::Entities::CraftableItemList&)
{
    return true;
}

bool DBCraftableItemList::Load(AB::Entities::CraftableItemList& il)
{
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    // Select by checking for the craftable item flag.
    // I'm not sure if it makes sense to index the item_flags column when we calculate with it...
    // Ahm, doesn't seem so. Anyway, this list does not change and doesn't need to be loaded frequently.
    query << "SELECT uuid, idx, type, item_flags, name, value FROM game_items WHERE item_flags & " <<
        static_cast<int>(AB::Entities::ItemFlagCraftable) << " = " << static_cast<int>(AB::Entities::ItemFlagCraftable);
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        il.items.push_back({ result->GetUInt("idx"),
            static_cast<AB::Entities::ItemType>(result->GetUInt("type")),
            result->GetString("name"),
            result->GetString("uuid"),
            static_cast<uint16_t>(result->GetUInt("value")),
            result->GetUInt("item_flags") });
    }
    return true;
}

bool DBCraftableItemList::Save(const AB::Entities::CraftableItemList&)
{
    return true;
}

bool DBCraftableItemList::Delete(const AB::Entities::CraftableItemList&)
{
    return true;
}

bool DBCraftableItemList::Exists(const AB::Entities::CraftableItemList&)
{
    return true;
}

}
