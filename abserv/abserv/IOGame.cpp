#include "stdafx.h"
#include "IOGame.h"

#include "DebugNew.h"

namespace DB {

bool IOGame::LoadGame(Game::Game* game, std::shared_ptr<DBResult> result)
{
    if (!result)
        return false;

    game->data_.id = result->GetUInt("id");
    game->data_.mapName = result->GetString("name");
    game->data_.mapFile = result->GetString("map");
    game->data_.type = static_cast<Game::GameType>(result->GetUInt("id"));

    return true;
}

bool IOGame::LoadGameByName(Game::Game* game, const std::string& name)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `type`, `map`, `landing` FROM `games` WHERE `name` = " <<
        db->EscapeString(name);

    return IOGame::LoadGame(game, db->StoreQuery(query.str()));
}

bool IOGame::LoadGameById(Game::Game* game, uint32_t gameId)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `type`, `map`, `landing` FROM `games` WHERE `id` = " <<
        gameId;

    return IOGame::LoadGame(game, db->StoreQuery(query.str()));
}

}
