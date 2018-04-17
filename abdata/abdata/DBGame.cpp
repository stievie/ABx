#include "stdafx.h"
#include "DBGame.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

uint32_t DBGame::Create(AB::Entities::Game&)
{
    // Do nothing
    return 0;
}

bool DBGame::Load(AB::Entities::Game& game)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `games` WHERE `id` = " << game.id;
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    game.name = result->GetString("name");
    game.type = static_cast<AB::Entities::GameType>(result->GetInt("type"));
    game.directory = result->GetString("directory");
    game.script = result->GetString("script_file");
    game.landing = result->GetUInt("landing") != 0;

    return true;
}

bool DBGame::Save(const AB::Entities::Game&)
{
    // Do nothing
    return true;
}

bool DBGame::Delete(const AB::Entities::Game&)
{
    // Do nothing
    return true;
}

}
