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


#include "DBMailList.h"

namespace DB {

bool DBMailList::Create(AB::Entities::MailList& ml)
{
    if (Utils::Uuid::IsEmpty(ml.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBMailList::Load(AB::Entities::MailList& ml)
{
    if (Utils::Uuid::IsEmpty(ml.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    ml.mails.clear();
    std::ostringstream query;
    query << "SELECT `uuid`, `from_name`, `subject`, `created`, `is_read` FROM `mails` WHERE `to_account_uuid` = " << db->EscapeString(ml.uuid);
    // Oldest first because the chat window scrolls down
    query << " ORDER BY `created` ASC";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        // Maybe no mails
        return true;

    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        ml.mails.push_back({
            result->GetString("uuid"),
            result->GetString("from_name"),
            result->GetString("subject"),
            result->GetLong("created"),
            result->GetUInt("is_read") != 0
        });
    }

    return true;
}

bool DBMailList::Save(const AB::Entities::MailList& ml)
{
    if (Utils::Uuid::IsEmpty(ml.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    // Do nothing
    return true;
}

bool DBMailList::Delete(const AB::Entities::MailList& ml)
{
    if (Utils::Uuid::IsEmpty(ml.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBMailList::Exists(const AB::Entities::MailList& ml)
{
    if (Utils::Uuid::IsEmpty(ml.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

}
