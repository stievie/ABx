#include "stdafx.h"
#include "DBCharacterList.h"
#include "Database.h"
#include "Subsystems.h"

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
