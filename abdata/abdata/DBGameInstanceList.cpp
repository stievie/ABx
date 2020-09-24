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

#include "DBGameInstanceList.h"

namespace DB {

bool DBGameInstanceList::Create(AB::Entities::GameInstanceList&)
{
    return true;
}

bool DBGameInstanceList::Load(AB::Entities::GameInstanceList& game)
{
    Database* db = GetSubsystem<Database>();

    static const std::string query = "SELECT uuid FROM instances WHERE is_running = 1 ORDER BY players DESC, start_time DESC";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query); result; result = result->Next())
    {
        game.uuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBGameInstanceList::Save(const AB::Entities::GameInstanceList&)
{
    return true;
}

bool DBGameInstanceList::Delete(const AB::Entities::GameInstanceList&)
{
    return true;
}

bool DBGameInstanceList::Exists(const AB::Entities::GameInstanceList&)
{
    return true;
}

}
