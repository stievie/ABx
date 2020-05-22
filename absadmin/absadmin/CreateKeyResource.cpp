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


#include "CreateKeyResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyList.h>

namespace Resources {

bool CreateKeyResource::CreateKey(const std::string& uuid,
    const std::string& keyType, const std::string& count,
    const std::string& keyStatus, const std::string& email,
    const std::string & descr)
{
    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountKey ak;
    if (!uuids::uuid(uuid).nil())
        ak.uuid = uuid;
    else
        ak.uuid = Utils::Uuid::New();
    if (dataClient->Read(ak))
        // Already exists
        return false;

    int iType = std::atoi(keyType.c_str());
    if (iType < 0 || iType > AB::Entities::KeyTypeCharSlot)
        return false;
    ak.type = static_cast<AB::Entities::AccountKeyType>(iType);
    int iCount = std::atoi(count.c_str());
    if (iCount < 0)
        return false;
    ak.total = static_cast<uint16_t>(iCount);
    int iStatus = std::atoi(keyStatus.c_str());
    if (iStatus < 0 || iStatus > AB::Entities::KeyStatusBanned)
        return false;
    ak.status = static_cast<AB::Entities::AccountKeyStatus>(iStatus);
    ak.email = email;
    ak.description = descr;

    bool succ = dataClient->Create(ak);
    if (succ)
    {
        AB::Entities::AccountKeyList akl;
        dataClient->Invalidate(akl);
    }
    return succ;
}

void CreateKeyResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(".json"));
    responseCookies_->Write(header);

    std::stringstream ss;
    ss << request_->content.rdbuf();

    json::JSON obj;
    SimpleWeb::CaseInsensitiveMultimap form = SimpleWeb::QueryString::parse(ss.str());
    auto uuidIt = form.find("uuid");
    auto keyTypeIt = form.find("key_type");
    auto countIt = form.find("count");
    auto keyStatusIt = form.find("key_status");
    auto emailIt = form.find("email");
    auto descrIt = form.find("description");
    if (uuidIt == form.end() || keyTypeIt == form.end() || countIt == form.end() ||
        keyStatusIt == form.end() || emailIt == form.end() || descrIt == form.end())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing field(s)";
    }
    else
    {
        if (!CreateKey((*uuidIt).second, (*keyTypeIt).second, (*countIt).second,
            (*keyStatusIt).second, (*emailIt).second, (*descrIt).second))
        {
            obj["status"] = "Failed";
            obj["message"] = "Failed";
        }
        else
        {
            obj["status"] = "OK";
        }
    }

    response->write(obj.dump(), header);
}

}
