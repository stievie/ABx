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

#include "DBPlayerItemList.h"
#include <sa/TemplateParser.h>

namespace DB {

bool DBPlayerItemList::Create(AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBPlayerItemList::Load(AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    sa::TemplateParser parser;
    sa::Template tokens = parser.Parse("SELECT `uuid` FROM `concrete_items` WHERE `player_uuid` = ${player_uuid} AND `deleted` = 0");
    if (il.storagePlace != AB::Entities::StoragePlace::None)
        parser.Append(" AND `storage_place` = ${storage_place}", tokens);
    const std::string query = tokens.ToString([db, &il](const sa::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::Token::Type::Expression:
            if (token.value == "player_uuid")
                return db->EscapeString(il.uuid);
            if (token.value == "storage_place")
                return std::to_string(static_cast<int>(il.storagePlace));
            ASSERT_FALSE();
        case sa::Token::Type::Quote:
            if (token.value == "`")
                return "\"";
            return token.value;
        default:
            return token.value;
        }
    });

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
    {
        il.itemUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBPlayerItemList::Save(const AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBPlayerItemList::Delete(const AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBPlayerItemList::Exists(const AB::Entities::PlayerItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
