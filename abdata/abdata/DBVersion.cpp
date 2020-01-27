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
#include "DBVersion.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBVersion::Create(AB::Entities::Version&)
{
    // Do nothing
    return true;
}

bool DBVersion::Load(AB::Entities::Version& v)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `versions` WHERE ";
    if (!Utils::Uuid::IsEmpty(v.uuid))
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (!v.name.empty())
        query << "`name` = " << db->EscapeString(v.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    v.uuid = result->GetString("uuid");
    v.name = result->GetString("name");
    v.value = result->GetUInt("value");
    v.isInternal = result->GetUInt("internal") != 0;

    return true;
}

bool DBVersion::Save(const AB::Entities::Version&)
{
    // Do nothing
    return true;
}

bool DBVersion::Delete(const AB::Entities::Version&)
{
    // Do nothing
    return true;
}

bool DBVersion::Exists(const AB::Entities::Version& v)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `versions` WHERE ";
    if (!Utils::Uuid::IsEmpty(v.uuid))
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (!v.name.empty())
        query << "`name` = " << db->EscapeString(v.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
