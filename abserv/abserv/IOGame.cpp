#include "stdafx.h"
#include "IOGame.h"

#include "DebugNew.h"

namespace DB {

bool IOGame::LoadGame(Game::Game* game, std::shared_ptr<DBResult> result)
{
    if (!result)
        return false;

    game->map_->data_.id = result->GetUInt("id");
    game->map_->data_.name = result->GetString("name");
    game->map_->data_.directory = result->GetString("directory");
    game->data_.scriptFile = result->GetString("script_file");
    game->data_.type = static_cast<Game::GameType>(result->GetUInt("id"));

    return true;
}

bool IOGame::LoadGameByName(Game::Game* game, const std::string& name)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `type`, `directory`, `script_file`, `landing` FROM `games` WHERE `name` = " <<
        db->EscapeString(name);

    return IOGame::LoadGame(game, db->StoreQuery(query.str()));
}

bool IOGame::LoadGameById(Game::Game* game, uint32_t gameId)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `type`, `directory`, `landing` FROM `games` WHERE `id` = " <<
        gameId;

    return IOGame::LoadGame(game, db->StoreQuery(query.str()));
}

std::string IOGame::GetLandingGame()
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `name`, `landing` FROM `games` WHERE `landing` = 1";
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (result)
        return result->GetString("name");
    return "";
}

Game::GameType IOGame::GetGameType(const std::string& mapName)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `type` FROM `games` WHERE `name` = " << db->EscapeString(mapName);
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (result)
        return static_cast<Game::GameType>(result->GetUInt("type"));
    return Game::GameTypeUnknown;
}

}
