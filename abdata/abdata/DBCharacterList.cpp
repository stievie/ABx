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
#include "DBCharacterList.h"

namespace DB {

bool DBCharacterList::Create(AB::Entities::CharacterList&)
{
    return true;
}

bool DBCharacterList::Load(AB::Entities::CharacterList& al)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `players`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        al.uuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBCharacterList::Save(const AB::Entities::CharacterList&)
{
    return true;
}

bool DBCharacterList::Delete(const AB::Entities::CharacterList&)
{
    return true;
}

bool DBCharacterList::Exists(const AB::Entities::CharacterList&)
{
    return true;
}

}
