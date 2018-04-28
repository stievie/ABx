#include "stdafx.h"
#include "IOGame.h"
#include "DataClient.h"
#include <AB/Entities/GameList.h>

#include "DebugNew.h"

namespace IO {

bool IOGame::LoadGame(AB::Entities::Game& game)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    return client->Read(game);

#if 0
    if (!result)
        return false;

    game->map_->data_.id = result->GetUInt("id");
    game->map_->data_.name = result->GetString("name");
    game->map_->data_.directory = result->GetString("directory");
    game->data_.scriptFile = result->GetString("script_file");
    game->data_.type = static_cast<AB::Data::GameType>(result->GetUInt("id"));

    return true;
#endif
}

bool IOGame::LoadGameByName(Game::Game* game, const std::string& name)
{
    game->data_.name = name;
    return LoadGame(game->data_);

#if 0
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `type`, `directory`, `script_file`, `landing` FROM `games` WHERE `name` = " <<
        db->EscapeString(name);

    return IOGame::LoadGame(game, db->StoreQuery(query.str()));
#endif
}

bool IOGame::LoadGameByUuid(Game::Game* game, const std::string& uuid)
{
    game->data_.uuid = uuid;
    return LoadGame(game->data_);

#if 0
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `type`, `directory`, `landing` FROM `games` WHERE `id` = " <<
        gameId;

    return IOGame::LoadGame(game, db->StoreQuery(query.str()));
#endif
}

std::string IOGame::GetLandingGame()
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::GameList gl;
    if (!client->Read(gl))
        return "";
    if (gl.gameUuids.size() == 0)
        return "";

    for (const std::string& uuid : gl.gameUuids)
    {
        AB::Entities::Game g;
        g.uuid = uuid;
        if (client->Read(g) && g.landing)
            return g.name;
    }
    return "";

#if 0
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `name`, `landing` FROM `games` WHERE `landing` = 1";
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (result)
        return result->GetString("name");
    return "";
#endif
}

AB::Entities::GameType IOGame::GetGameType(const std::string& mapName)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Game g;
    g.name = mapName;
    if (!client->Read(g))
        return AB::Entities::GameTypeUnknown;
    return g.type;

#if 0
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `type` FROM `games` WHERE `name` = " << db->EscapeString(mapName);
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (result)
        return static_cast<AB::Data::GameType>(result->GetUInt("type"));
    return AB::Data::GameType::GameTypeUnknown;
#endif
}

std::vector<AB::Entities::Game> IOGame::GetGameList()
{
    std::vector<AB::Entities::Game> result;

    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::GameList gl;
    if (!client->Read(gl))
        return result;

    for (const std::string& uuid : gl.gameUuids)
    {
        AB::Entities::Game g;
        g.uuid = uuid;
        if (!client->Read(g))
            continue;
        result.push_back(g);
    }

    if (result.size() == 0)
        LOG_WARNING << "No Games found!" << std::endl;
    return result;
#if 0
    Database* db = Database::Instance();

    std::vector<AB::Data::GameData> result;

    std::ostringstream query;
    std::shared_ptr<DBResult> dbResult;
    query << "SELECT `id`, `name`, `type` FROM games";
    if (type != AB::Data::GameType::GameTypeUnknown)
        query << " WHERE `type` = " << static_cast<uint32_t>(type);
    for (dbResult = db->StoreQuery(query.str()); dbResult; dbResult = dbResult->Next())
    {
        AB::Data::GameData game
        {
            dbResult->GetUInt("id"),
            dbResult->GetString("name"),
            static_cast<AB::Data::GameType>(dbResult->GetUInt("type"))
        };
        result.push_back(game);
    }
    return result;
#endif
}

}
