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

#include "NewsJsonResource.h"
#include "ContentTypes.h"
#include "Application.h"
#include <uuid.h>
#include <sa/time.h>
#include <AB/Entities/News.h>
#include <AB/Entities/NewsList.h>
#include <AB/Entities/Account.h>
#include <abscommon/StringUtils.h>

namespace Resources {

void NewsJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::Normal))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    json::JSON obj;
    obj = json::Array();

    auto* dataClient = GetSubsystem<IO::DataClient>();
    auto all = GetQueryValue("all");

    if (all.has_value())
    {
        if (!IsAllowed(AB::Entities::AccountType::Gamemaster))
        {
            response->write(SimpleWeb::StatusCode::client_error_unauthorized,
                "Unauthorized");
            return;
        }

        AB::Entities::AllNewsList list;
        if (!dataClient->Read(list))
        {
            response->write(SimpleWeb::StatusCode::client_error_not_found,
                "Not found");
            return;
        }
        for (const auto& uuid : list.uuids)
        {
            AB::Entities::News news;
            news.uuid = uuid;
            if (!dataClient->Read(news))
                continue;

            auto newsJson = json::Object();
            newsJson["uuid"] = news.uuid;
            newsJson["created"] = news.created;
            newsJson["body"] = news.body;

            obj.append(newsJson);
        }
    }
    else
    {
        AB::Entities::LatestNewsList list;
        if (!dataClient->Read(list))
        {
            response->write(SimpleWeb::StatusCode::client_error_not_found,
                "Not found");
            return;
        }
        for (const auto& uuid : list.uuids)
        {
            AB::Entities::News news;
            news.uuid = uuid;
            if (!dataClient->Read(news))
                continue;

            auto newsJson = json::Object();
            newsJson["uuid"] = news.uuid;
            newsJson["created"] = news.created;
            newsJson["body"] = news.body;

            obj.append(newsJson);
        }
    }

    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));
    Send(obj.dump(), response);
}

}
