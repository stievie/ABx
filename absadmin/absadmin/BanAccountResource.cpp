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


#include "BanAccountResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include <AB/Entities/Ban.h>
#include <sa/ScopeGuard.h>
#include <abscommon/UuidUtils.h>
#include <sa/StringTempl.h>
#include <AB/Entities/AccountBan.h>
#include <AB/Entities/AccountBanList.h>
#include <limits>

namespace Resources {

void BanAccountResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));

    json::JSON obj;

    sa::ScopeGuard send([&]()
    {
        Send(obj.dump(), response);
    });

    auto uuidField = GetFormField("uuid");
    if (!uuidField.has_value())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing UUID field";
        return;
    }

    std::string accountUuid = uuidField.value();
    AB::Entities::Ban ban;
    ban.uuid = Utils::Uuid::New();
    auto expiresField = GetFormField("expires");
    if (!expiresField.has_value())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing expires field";
        return;
    }
    auto expires = sa::to_number<int>(expiresField.value());
    if (!expires.has_value())
    {
        obj["status"] = "Failed";
        obj["message"] = "Wrong expires value";
        return;
    }
    switch (expires.value())
    {
    case 1:
        ban.expires = sa::time::tick() + sa::time::WEEK;
        break;
    case 2:
        ban.expires = sa::time::tick() + sa::time::MONTH;
        break;
    case 3:
        ban.expires = sa::time::tick() + (3 * sa::time::MONTH);
        break;
    case 4:
        ban.expires = sa::time::tick() + sa::time::YEAR;
        break;
    case 9:
        ban.expires = std::numeric_limits<int64_t>::max();
        break;
    default:
        ban.expires = sa::time::tick() + expires.value();
        break;
    }
    ban.added = sa::time::tick();
    ban.reason = AB::Entities::BanReasonOther;
    ban.active = true;

    auto adminIt = session_->values_.find(sa::StringHashRt("account_uuid"));
    if (adminIt != session_->values_.end())
        ban.adminUuid = adminIt->second.GetString();

    auto commentField = GetFormField("comment");
    if (commentField.has_value())
    {
        ban.comment = commentField.value();
    }

    auto* dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Create(ban))
    {
        obj["status"] = "Failed";
        obj["message"] = "Failed creating ban";
        return;
    }

    AB::Entities::AccountBan accBan;
    accBan.uuid = Utils::Uuid::New();
    accBan.accountUuid = accountUuid;
    accBan.banUuid = ban.uuid;
    if (!dataClient->Create(accBan))
    {
        obj["status"] = "Failed";
        obj["message"] = "Failed creating account ban";
        return;
    }

    AB::Entities::AccountBanList banList;
    banList.uuid = accountUuid;
    dataClient->Invalidate(banList);

    obj["status"] = "OK";
}

}
