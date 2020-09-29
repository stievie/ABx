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

#include "AccountBansJsonResource.h"
#include "Application.h"
#include "Version.h"
#include <AB/Entities/AccountBanList.h>
#include <AB/Entities/AccountList.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Ban.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>
#include "ContentTypes.h"

namespace Resources {

void AccountBansJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        Redirect(response, "/");
        return;
    }

    auto uuid = GetQueryValue("id");
    if (!uuid.has_value())
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    json::JSON obj = json::Array();

    AB::Entities::AccountBanList bans;
    bans.uuid = uuid.value();
    if (dataClient->Read(bans))
    {
        for (const auto& ban_uuid : bans.uuids)
        {
            AB::Entities::Ban ban;
            ban.uuid = ban_uuid;
            if (!dataClient->Read(ban))
                continue;

            auto banJson = json::Object();
            AB::Entities::Account adminAccount;
            adminAccount.uuid = ban.adminUuid;
            if (!Utils::Uuid::IsEmpty(ban.adminUuid))
                dataClient->Read(adminAccount);

            banJson["uuid"] = ban.uuid;
            banJson["active"] = ban.active;
            banJson["added"] = ban.added;
            banJson["expires"] = ban.expires == std::numeric_limits<int64_t>::max() ? 0 : ban.expires;
            banJson["comment"] = ban.comment;
            banJson["hits"] = ban.hits;
            banJson["admin_uuid"] = ban.adminUuid;
            banJson["admin_name"] = adminAccount.name.empty() ? "Unknown" : adminAccount.name;
            obj.append(banJson);
        }
    }
    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));
    Send(obj.dump(), response);
}

}
