/**
 * Copyright 2020 Stefan Ascher
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

#include "GamesJsonResource.h"
#include "ContentTypes.h"
#include "Application.h"
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/GameInstanceList.h>
#include <uuid.h>
#include <sa/time.h>

namespace Resources {

void GamesJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::GameInstanceList al;
    if (!dataClient->Read(al))
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found");
        return;
    }

    json::JSON obj;
    // https://raw.githubusercontent.com/fooplugins/FooTable/V3/docs/content/rows.json
    // https://datatables.net/examples/ajax/objects.html
    obj = json::Array();

    for (const auto& uuid : al.uuids)
    {
        AB::Entities::GameInstance inst;
        inst.uuid = uuid;
        if (!dataClient->Read(inst))
            continue;

        AB::Entities::Game game;
        game.uuid = inst.gameUuid;
        if (uuids::uuid(game.uuid).nil() || !dataClient->Read(game))
        {
            game.name = "None";
        }
        AB::Entities::Service serv;
        serv.uuid = inst.serverUuid;
        if (uuids::uuid(serv.uuid).nil() || !dataClient->Read(serv))
        {
            serv.name = "None";
        }

        auto instanceJson = json::Object();
        instanceJson["uuid"] = inst.uuid;
        instanceJson["type"] = static_cast<int>(game.type);
        instanceJson["start"] = inst.startTime;
        if (inst.stopTime == 0)
            instanceJson["time"] = (sa::time::tick() - inst.startTime);
        else
            instanceJson["time"] = (inst.stopTime - inst.startTime);
        instanceJson["players"] = inst.players;
        instanceJson["name"] = Utils::XML::Escape(game.name);
        instanceJson["number"] = inst.number;
        instanceJson["current_server"] = inst.serverUuid;
        instanceJson["current_server_name"] = Utils::XML::Escape(serv.name);
        instanceJson["current_server_link"] = "<a href=\"service?id=" + inst.serverUuid + "\">" + Utils::XML::Escape(serv.name) + "</a>";
        instanceJson["current_instance"] = inst.uuid;
        instanceJson["current_instance_link"] = "<a href=\"game?id=" + inst.uuid + "\">" + Utils::XML::Escape(game.name) + "</a>";

        obj.append(instanceJson);
    }

    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));
    Send(obj.dump(), response);
}

}
