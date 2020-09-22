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


#include "AccountResource.h"
#include "Application.h"
#include "Version.h"
#include <AB/Entities/AccountList.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>

namespace Resources {

bool AccountResource::GetContext(LuaContext& objects)
{
    if (!TemplateResource::GetContext(objects))
        return false;

    if (id_.empty() || uuids::uuid(id_).nil())
        return false;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = id_;
    if (!dataClient->Read(acc))
        return false;

    kaguya::State& state = objects.GetState();

    state["uuid"] = acc.uuid;
    state["name"] = Utils::XML::Escape(acc.name);
    state["type"] = static_cast<int>(acc.type);

    state["type_unknown"] = acc.type == AB::Entities::AccountType::Unknown;
    state["type_normnal"] = acc.type == AB::Entities::AccountType::Normal;
    state["type_tutor"] = acc.type == AB::Entities::AccountType::Tutor;
    state["type_sentutor"] = acc.type == AB::Entities::AccountType::SeniorTutor;
    state["type_gm"] = acc.type == AB::Entities::AccountType::Gamemaster;
    state["type_god"] = acc.type == AB::Entities::AccountType::God;

    state["status"] = static_cast<int>(acc.status);
    state["status_unknown"] = acc.status == AB::Entities::AccountStatusUnknown;
    state["status_activated"] = acc.status == AB::Entities::AccountStatusActivated;
    state["status_deleted"] = acc.status == AB::Entities::AccountStatusDeleted;

    state["online_offline"] = acc.onlineStatus == AB::Entities::OnlineStatusOffline;
    state["online_away"] = acc.onlineStatus == AB::Entities::OnlineStatusAway;
    state["online_dnd"] = acc.onlineStatus == AB::Entities::OnlineStatusDoNotDisturb;
    state["online_online"] = acc.onlineStatus == AB::Entities::OnlineStatusOnline;
    state["online_invisible"] = acc.onlineStatus == AB::Entities::OnlineStatusInvisible;
    state["is_online"] = acc.onlineStatus != AB::Entities::OnlineStatusOffline;

    AB::Entities::Character ch;
    ch.uuid = acc.currentCharacterUuid;
    if (uuids::uuid(ch.uuid).nil() || !dataClient->Read(ch))
    {
        ch.name = "None";
    }
    state["curr_char"] = ch.uuid;
    state["curr_char_name"] = ch.name;

    AB::Entities::Service serv;
    serv.uuid = acc.currentServerUuid;
    if (uuids::uuid(serv.uuid).nil() || !dataClient->Read(serv))
    {
        serv.name = "None";
    }
    state["curr_server"] = serv.uuid;
    state["curr_server_name"] = serv.name;

    std::string instanceName = "None";
    std::string instanceUuid = Utils::Uuid::EMPTY_UUID;
    if (acc.onlineStatus != AB::Entities::OnlineStatusOffline)
    {
        AB::Entities::GameInstance gi;
        gi.uuid = ch.instanceUuid;
        if (!uuids::uuid(gi.uuid).nil() && dataClient->Read(gi))
        {
            instanceUuid = gi.uuid;
            AB::Entities::Game game;
            // If there is an instance the Game UUID must not be nil, so skip checking
            game.uuid = gi.gameUuid;
            if (dataClient->Read(game))
                instanceName = game.name;
        }
    }
    state["curr_inst"] = instanceUuid;
    state["curr_inst_name"] = instanceName;

    return true;
}

AccountResource::AccountResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request),
    id_("")
{
    template_ = "../templates/account.lpp";

    SimpleWeb::CaseInsensitiveMultimap form = request->parse_query_string();
    auto it = form.find("id");
    if (it != form.end())
        id_ = (*it).second;

}

void AccountResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
