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

#include "stdafx.h"
#include "DBItem.h"

namespace DB {

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

    std::ostringstream query;
    query << "SELECT * FROM `game_items` WHERE ";
    if (!Utils::Uuid::IsEmpty(item.uuid))
        query << "`uuid` = " << db->EscapeString(item.uuid);
    else if (item.index != 0)
        query << "`idx` = " << item.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    item.uuid = result->GetString("uuid");
    item.index = result->GetUInt("idx");
    item.model_class = static_cast<AB::Entities::ModelClass>(result->GetUInt("model_class"));
    item.name = result->GetString("name");
    item.script = result->GetString("schript_file");
    item.objectFile = result->GetString("object_file");
    item.iconFile = result->GetString("icon_file");
    item.type = static_cast<AB::Entities::ItemType>(result->GetUInt("type"));
    item.belongsTo = static_cast<AB::Entities::ItemType>(result->GetUInt("belongs_to"));
    item.stackAble = result->GetUInt("stack_able") != 0;
    item.value = static_cast<uint16_t>(result->GetUInt("value"));
    item.spawnItemUuid = result->GetString("spawn_item_uuid");
    item.actorScript = result->GetString("actor_script");

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

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_items` WHERE ";
    if (!Utils::Uuid::IsEmpty(item.uuid))
        query << "`uuid` = " << db->EscapeString(item.uuid);
    else if (item.index != 0)
        query << "`idx` = " << item.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
