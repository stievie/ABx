#include "stdafx.h"
#include "AccountResource.h"
#include "StringHash.h"
#include "Application.h"
#include "Version.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "StringUtils.h"
#include <AB/Entities/AccountList.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>
#include "UuidUtils.h"
#include "Xml.h"

namespace Resources {

bool AccountResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;

    if (id_.empty() || uuids::uuid(id_).nil())
        return false;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = id_;
    if (!dataClient->Read(acc))
        return false;

    objects["uuid"] = acc.uuid;
    objects["name"] = Utils::XML::Escape(acc.name);
    objects["type"] = static_cast<int>(acc.type);

    objects["type_unknown"] = acc.type == AB::Entities::AccountTypeUnknown;
    objects["type_normnal"] = acc.type == AB::Entities::AccountTypeNormal;
    objects["type_tutor"] = acc.type == AB::Entities::AccountTypeTutor;
    objects["type_sentutor"] = acc.type == AB::Entities::AccountTypeSeniorTutor;
    objects["type_gm"] = acc.type == AB::Entities::AccountTypeGamemaster;
    objects["type_god"] = acc.type == AB::Entities::AccountTypeGod;

    objects["status"] = static_cast<int>(acc.status);
    objects["status_unknown"] = acc.status == AB::Entities::AccountStatusUnknown;
    objects["status_activated"] = acc.status == AB::Entities::AccountStatusActivated;
    objects["status_deleted"] = acc.status == AB::Entities::AccountStatusDeleted;

    objects["online_offline"] = acc.onlineStatus == AB::Entities::OnlineStatusOffline;
    objects["online_away"] = acc.onlineStatus == AB::Entities::OnlineStatusAway;
    objects["online_dnd"] = acc.onlineStatus == AB::Entities::OnlineStatusDoNotDisturb;
    objects["online_online"] = acc.onlineStatus == AB::Entities::OnlineStatusOnline;
    objects["online_invisible"] = acc.onlineStatus == AB::Entities::OnlineStatusInvisible;
    objects["is_online"] = acc.onlineStatus != AB::Entities::OnlineStatusOffline;

    AB::Entities::Character ch;
    ch.uuid = acc.currentCharacterUuid;
    if (uuids::uuid(ch.uuid).nil() || !dataClient->Read(ch))
    {
        ch.name = "None";
    }
    objects["curr_char"] = ch.uuid;
    objects["curr_char_name"] = ch.name;

    AB::Entities::Service serv;
    serv.uuid = acc.currentServerUuid;
    if (uuids::uuid(serv.uuid).nil() || !dataClient->Read(serv))
    {
        serv.name = "None";
    }
    objects["curr_server"] = serv.uuid;
    objects["curr_server_name"] = serv.name;

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
    objects["curr_inst"] = instanceUuid;
    objects["curr_inst_name"] = instanceName;

    return true;
}

AccountResource::AccountResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request),
    id_("")
{
    template_ = "../templates/account.html";

    footerScripts_.push_back("vendors/gauge.js/dist/gauge.min.js");

    SimpleWeb::CaseInsensitiveMultimap form = request->parse_query_string();
    auto it = form.find("id");
    if (it != form.end())
        id_ = (*it).second;

}

void AccountResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
