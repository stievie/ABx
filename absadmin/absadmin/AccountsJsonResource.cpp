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
#include "AccountsJsonResource.h"
#include "ContentTypes.h"
#include "Application.h"
#include <AB/Entities/AccountList.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>
#include <uuid.h>

namespace Resources {

void AccountsJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountList al;
    if (!dataClient->Read(al))
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found");
        return;
    }

    json::JSON obj;
    // https://datatables.net/examples/data_sources/ajax.html
    // https://datatables.net/examples/ajax/objects.html
    obj["data"] = json::Array();

    for (const auto& uuid : al.uuids)
    {
        AB::Entities::Account a;
        a.uuid = uuid;
        if (!dataClient->Read(a))
            continue;

        AB::Entities::Character ch;
        ch.uuid = a.currentCharacterUuid;
        if (uuids::uuid(ch.uuid).nil() || !dataClient->Read(ch))
        {
            ch.name = "None";
        }
        AB::Entities::Service serv;
        serv.uuid = a.currentServerUuid;
        if (uuids::uuid(serv.uuid).nil() || !dataClient->Read(serv))
        {
            serv.name = "None";
        }
        std::string instanceName = "None";
        if (a.onlineStatus != AB::Entities::OnlineStatusOffline)
        {
            AB::Entities::GameInstance gi;
            gi.uuid = ch.instanceUuid;
            if (!uuids::uuid(gi.uuid).nil() && dataClient->Read(gi))
            {
                AB::Entities::Game game;
                // If there is an instance the Game UUID must not be nil, so skip checking
                game.uuid = gi.gameUuid;
                if (dataClient->Read(game))
                    instanceName = game.name;
            }
        }

        auto account = json::Object();
        account["uuid"] = a.uuid;
        account["type"] = static_cast<int>(a.type);
        account["status"] = static_cast<int>(a.status);
        account["name"] = Utils::XML::Escape(a.name);
        account["name_link"] = "<a href=\"account?id=" + a.uuid + "\">" + Utils::XML::Escape(a.name) + "</a>";;
        account["email"] = a.email;
        account["current_server"] = a.currentServerUuid;
        account["current_server_name"] = Utils::XML::Escape(serv.name);
        account["current_server_link"] = "<a href=\"service?id=" + a.currentServerUuid + "\">" + Utils::XML::Escape(serv.name) + "</a>";
        account["current_character"] = a.currentCharacterUuid;
        account["current_character_link"] = "<a href=\"character?id=" + a.currentCharacterUuid + "\">" + Utils::XML::Escape(ch.name) + "</a>";
        account["current_instance"] = ch.instanceUuid;
        account["current_instance_link"] = "<a href=\"game?id=" + ch.instanceUuid + "\">" + Utils::XML::Escape(instanceName) + "</a>";
        account["online"] = static_cast<int>(a.onlineStatus);

        obj["data"].append(account);
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(".json"));
    responseCookies_->Write(header);
    response->write(obj.dump(), header);
}

}
