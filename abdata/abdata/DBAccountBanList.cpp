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

#include "DBAccountBanList.h"
#include <sa/TemplateParser.h>

namespace DB {

bool DBAccountBanList::Create(AB::Entities::AccountBanList& li)
{
    if (Utils::Uuid::IsEmpty(li.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountBanList::Load(AB::Entities::AccountBanList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    static constexpr const char* SQL = "SELECT ban_uuid FROM account_bans WHERE account_uuid = ${account_uuid}";
    const std::string query = sa::templ::Parser::Evaluate(SQL, [db, &il](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "account_uuid")
                return db->EscapeString(il.uuid);
            LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
            return "";
        default:
            return token.value;
        }
    });

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
    {
        il.uuids.push_back(result->GetString("ban_uuid"));
    }
    return true;
}

bool DBAccountBanList::Save(const AB::Entities::AccountBanList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountBanList::Delete(const AB::Entities::AccountBanList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

bool DBAccountBanList::Exists(const AB::Entities::AccountBanList& il)
{
    if (Utils::Uuid::IsEmpty(il.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    return true;
}

}
