#include "stdafx.h"
#include "AccountsJsonResource.h"
#include "StringHash.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "Application.h"
#include "StringUtils.h"
#include <AB/Entities/AccountList.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>

namespace Resources {

void AccountsJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
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
        if (!dataClient->Read(ch))
        {
            ch.name = "None";
        }
        AB::Entities::Service serv;
        serv.uuid = a.currentServerUuid;
        if (!dataClient->Read(serv))
        {
            serv.name = "None";
        }
        std::string instanceName = "None";
        if (a.onlineStatus != AB::Entities::OnlineStatusOffline)
        {
            AB::Entities::GameInstance gi;
            gi.uuid = ch.instanceUuid;
            if (dataClient->Read(gi))
            {
                AB::Entities::Game game;
                game.uuid = gi.gameUuid;
                if (dataClient->Read(game))
                    instanceName = game.name;
            }
        }

        auto account = json::Object();
        account["uuid"] = a.uuid;
        account["type"] = static_cast<int>(a.type);
        account["status"] = static_cast<int>(a.status);
        account["name"] = a.name;
        account["name_link"] = "<a href=\"account?id=" + a.uuid + "\">" + a.name + "</a>";;
        account["email"] = a.email;
        account["current_server"] = a.currentServerUuid;
        account["current_server_name"] = serv.name;
        account["current_server_link"] = "<a href=\"service?id=" + a.currentServerUuid + "\">" + serv.name + "</a>";
        account["current_character"] = a.currentCharacterUuid;
        account["current_character_link"] = "<a href=\"character?id=" + a.currentCharacterUuid + "\">" + ch.name + "</a>";
        account["current_instance"] = ch.instanceUuid;
        account["current_instance_link"] = "<a href=\"game?id=" + ch.instanceUuid + "\">" + instanceName + "</a>";
        account["online"] = static_cast<int>(a.onlineStatus);

        obj["data"].append(account);
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(Utils::GetFileExt(".json")));
    responseCookies_->Write(header);
    response->write(obj.dump(), header);
}

}
