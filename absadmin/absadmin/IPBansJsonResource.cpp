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


#include "IPBansJsonResource.h"
#include "ContentTypes.h"
#include "Application.h"
#include <uuid.h>
#include <sa/time.h>
#include <AB/Entities/IpBanList.h>
#include <AB/Entities/IpBan.h>
#include <AB/Entities/Ban.h>
#include <AB/Entities/Account.h>
#include <abscommon/StringUtils.h>

namespace Resources {

void IPBansJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::IpBanList list;
    if (!dataClient->Read(list))
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found");
        return;
    }

    json::JSON obj;
    obj = json::Array();

    for (const auto& uuid : list.uuids)
    {
        AB::Entities::IpBan ipban;
        ipban.uuid = uuid;
        if (!dataClient->Read(ipban))
            continue;

        AB::Entities::Ban ban;
        ban.uuid = ipban.banUuid;
        if (!dataClient->Read(ban))
            continue;

        AB::Entities::Account adminAccount;
        adminAccount.uuid = ban.adminUuid;
        if (!Utils::Uuid::IsEmpty(ban.adminUuid))
            dataClient->Read(adminAccount);

        auto banJson = json::Object();
        banJson["uuid"] = ipban.banUuid;
        banJson["ipban_uuid"] = ipban.uuid;
        banJson["ip"] = Utils::ConvertIPToString(ipban.ip);
        banJson["mask"] = Utils::ConvertIPToString(ipban.mask);
        banJson["active"] = ban.active;
        banJson["added"] = ban.added;
        banJson["expires"] = ban.expires == std::numeric_limits<int64_t>::max() ? 0 : ban.expires;
        banJson["comment"] = ban.comment;
        banJson["hits"] = ban.hits;
        banJson["admin_uuid"] = ban.adminUuid;
        banJson["admin_name"] = adminAccount.name.empty() ? "Unknown" : adminAccount.name;

        obj.append(banJson);
    }

    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));
    Send(obj.dump(), response);
}

}
