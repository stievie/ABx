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

#include "DBAccountItemList.h"
#include <sa/TemplateParser.h>

namespace DB {

template<AB::Entities::StoragePlace _Place>
static bool LoadList(AB::Entities::_AccountItemList<_Place>& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT uuid FROM concrete_items WHERE account_uuid = ${player_uuid} AND deleted = 0");
    if (il.storagePlace != AB::Entities::StoragePlace::None)
        parser.Append(" AND storage_place = ${storage_place}", tokens);
    const std::string query = tokens.ToString([db, &il](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "player_uuid")
                return db->EscapeString(il.uuid);
            if (token.value == "storage_place")
                return std::to_string(static_cast<int>(il.storagePlace));
            LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
            return "";
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

bool DBAccountItemList::Create(AB::Entities::AccountItemList& li)
{
    if (Utils::Uuid::IsEmpty(li.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Load(AB::Entities::AccountItemList& il)
{
    return LoadList<AB::Entities::AccountItemList::Place>(il);
}

bool DBAccountItemList::Save(const AB::Entities::AccountItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Delete(const AB::Entities::AccountItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Exists(const AB::Entities::AccountItemList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Create(AB::Entities::ChestItems& li)
{
    if (Utils::Uuid::IsEmpty(li.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Load(AB::Entities::ChestItems& il)
{
    return LoadList<AB::Entities::ChestItems::Place>(il);
}

bool DBAccountItemList::Save(const AB::Entities::ChestItems& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Delete(const AB::Entities::ChestItems& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountItemList::Exists(const AB::Entities::ChestItems& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
