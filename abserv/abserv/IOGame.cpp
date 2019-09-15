#include "stdafx.h"
#include "IOGame.h"
#include "DataClient.h"
#include <AB/Entities/GameList.h>
#include "Logger.h"
#include "UuidUtils.h"
#include "Subsystems.h"

namespace IO {

bool IOGame::LoadGame(AB::Entities::Game& game)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    return client->Read(game);
}

bool IOGame::LoadGameByName(Game::Game& game, const std::string& name)
{
    game.data_.name = name;
    return LoadGame(game.data_);
}

bool IOGame::LoadGameByUuid(Game::Game& game, const std::string& uuid)
{
    game.data_.uuid = uuid;
    return LoadGame(game.data_);
}

std::string IOGame::GetLandingGameUuid()
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::GameList gl;
    if (!client->Read(gl))
    {
        LOG_ERROR << "Error reading game list" << std::endl;
        return Utils::Uuid::EMPTY_UUID;
    }
    if (gl.gameUuids.size() == 0)
    {
        LOG_ERROR << "Game list is empty" << std::endl;
        return Utils::Uuid::EMPTY_UUID;
    }

    for (const std::string& uuid : gl.gameUuids)
    {
        AB::Entities::Game g;
        g.uuid = uuid;
        if (client->Read(g) && g.landing)
            return g.uuid;
    }
    LOG_ERROR << "No landing game found" << std::endl;
    return Utils::Uuid::EMPTY_UUID;
}

AB::Entities::GameType IOGame::GetGameType(const std::string& mapUuid)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Game g;
    g.uuid = mapUuid;
    if (!client->Read(g))
    {
        LOG_ERROR << "Error reading game with UUID " << mapUuid << std::endl;
        return AB::Entities::GameTypeUnknown;
    }
    return g.type;
}

std::vector<AB::Entities::Game> IOGame::GetGameList()
{
    std::vector<AB::Entities::Game> result;

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::GameList gl;
    if (!client->Read(gl))
    {
        LOG_ERROR << "Error reading game list" << std::endl;
        return result;
    }

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
}

std::string IOGame::GetGameUuidFromName(const std::string& name)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Game g;
    g.name = name;
    if (client->Read(g))
        return g.uuid;

    return Utils::Uuid::EMPTY_UUID;
}

}
