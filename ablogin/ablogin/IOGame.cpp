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
#include "IOGame.h"
#include "DataClient.h"
#include <AB/Entities/GameList.h>
#include "Logger.h"
#include "Application.h"
#include "UuidUtils.h"
#include "Subsystems.h"

namespace IO {

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

AB::Entities::GameType IOGame::GetGameType(const std::string& mapName)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Game g;
    g.name = mapName;
    if (!client->Read(g))
    {
        LOG_ERROR << "Error reading game " << mapName << std::endl;
        return AB::Entities::GameTypeUnknown;
    }
    return g.type;
}

std::vector<AB::Entities::Game> IOGame::GetGameList(const std::set<AB::Entities::GameType>& types)
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
        if (types.empty() || types.find(g.type) != types.end())
            result.push_back(g);
    }

    if (result.size() == 0)
        LOG_WARNING << "No Games found!" << std::endl;
    return result;
}

}
