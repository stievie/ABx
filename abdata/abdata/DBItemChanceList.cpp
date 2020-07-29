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

#include "DBItemChanceList.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::ItemChanceList& il, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "map_uuid")
            return db->EscapeString(il.uuid);
        if (token.value == "empty_map_uuid")
            return db->EscapeString(Utils::Uuid::EMPTY_UUID);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBItemChanceList::Create(AB::Entities::ItemChanceList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBItemChanceList::Load(AB::Entities::ItemChanceList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT item_uuid, chance, can_drop FROM game_item_chances WHERE "
        "map_uuid = ${map_uuid} OR map_uuid = ${empty_map_uuid}";

    static const std::string query = sa::templ::Parser::Evaluate(SQL, std::bind(&PlaceholderCallback, db, il, std::placeholders::_1));
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
    {
        il.items.push_back({
            result->GetString("item_uuid"),
            result->GetUInt("chance"),
            result->GetUInt("can_drop") == 1
        });
    }
    return true;
}

bool DBItemChanceList::Save(const AB::Entities::ItemChanceList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBItemChanceList::Delete(const AB::Entities::ItemChanceList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBItemChanceList::Exists(const AB::Entities::ItemChanceList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
