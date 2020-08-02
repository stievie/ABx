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

#include "DBItem.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Item& item, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(item.uuid);
        if (token.value == "idx")
            return std::to_string(item.index);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBItem::Create(AB::Entities::Item& item)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBItem::Load(AB::Entities::Item& item)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT * FROM game_items WHERE uuid= ${uuid}";
    static constexpr const char* SQL_INDEX = "SELECT * FROM game_items WHERE idx = ${index}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(item.uuid))
        sql = SQL_UUID;
    else if (!item.index != 0)
        sql = SQL_INDEX;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    item.uuid = result->GetString("uuid");
    item.index = result->GetUInt("idx");
    item.model_class = static_cast<AB::Entities::ModelClass>(result->GetUInt("model_class"));
    item.name = result->GetString("name");
    item.script = result->GetString("script_file");
    item.objectFile = result->GetString("object_file");
    item.iconFile = result->GetString("icon_file");
    item.type = static_cast<AB::Entities::ItemType>(result->GetUInt("type"));
    item.belongsTo = static_cast<AB::Entities::ItemType>(result->GetUInt("belongs_to"));
    item.value = static_cast<uint16_t>(result->GetUInt("value"));
    item.spawnItemUuid = result->GetString("spawn_item_uuid");
    item.actorScript = result->GetString("actor_script");
    item.itemFlags = result->GetUInt("item_flags");

    return true;
}

bool DBItem::Save(const AB::Entities::Item& item)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBItem::Delete(const AB::Entities::Item& item)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(item.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBItem::Exists(const AB::Entities::Item& item)
{
    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL_UUID = "SELECT COUNT(*) AS count FROM game_items WHERE uuid= ${uuid}";
    static constexpr const char* SQL_INDEX = "SELECT COUNT(*) AS count FROM game_items WHERE idx = ${index}";

    const char* sql = nullptr;
    if (!Utils::Uuid::IsEmpty(item.uuid))
        sql = SQL_UUID;
    else if (!item.index != 0)
        sql = SQL_INDEX;
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    const std::string query = sa::templ::Parser::Evaluate(sql, std::bind(&PlaceholderCallback, db, item, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
