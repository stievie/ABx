#include "stdafx.h"
#include "DBGame.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBGame::Create(AB::Entities::Game&)
{
    // Do nothing
    return true;
}

bool DBGame::Load(AB::Entities::Game& game)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_maps` WHERE ";
    if (!game.uuid.empty() && !uuids::uuid(game.uuid).nil())
        query << "`uuid` = " << db->EscapeString(game.uuid);
    else if (!game.name.empty())
        query << "`name` = " << db->EscapeString(game.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    game.uuid = result->GetString("uuid");
    game.name = result->GetString("name");
    game.type = static_cast<AB::Entities::GameType>(result->GetInt("type"));
    game.directory = result->GetString("directory");
    game.script = result->GetString("script_file");
    game.landing = result->GetUInt("landing") != 0;
    game.partySize = static_cast<uint8_t>(result->GetUInt("party_size"));
    game.mapCoordX = result->GetUInt("map_coord_x");
    game.mapCoordY = result->GetUInt("map_coord_y");

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

bool DBGame::Exists(const AB::Entities::Game& game)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_maps` WHERE ";
    if (!game.uuid.empty() && !uuids::uuid(game.uuid).nil())
        query << "`uuid` = " << db->EscapeString(game.uuid);
    else if (!game.name.empty())
        query << "`name` = " << db->EscapeString(game.name);
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
