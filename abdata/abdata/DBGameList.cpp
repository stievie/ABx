#include "stdafx.h"
#include "DBGameList.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

bool DBGameList::Create(AB::Entities::GameList&)
{
    return true;
}

bool DBGameList::Load(AB::Entities::GameList& game)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_maps`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        game.gameUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBGameList::Save(const AB::Entities::GameList&)
{
    return true;
}

bool DBGameList::Delete(const AB::Entities::GameList&)
{
    return true;
}

bool DBGameList::Exists(const AB::Entities::GameList&)
{
    return true;
}

}
