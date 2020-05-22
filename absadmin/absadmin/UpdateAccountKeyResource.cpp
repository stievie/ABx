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


#include "UpdateAccountKeyResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include <AB/Entities/AccountKey.h>

namespace Resources {

bool UpdateAccountKeyResource::Update(const std::string& uuid,
    const std::string& filed,
    const std::string& value)
{
    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountKey ak;
    ak.uuid = uuid;
    if (!dataClient->Read(ak))
        return false;

    if (filed.compare("type") == 0)
    {
        int iType = std::atoi(value.c_str());
        if (iType < 0 || iType > AB::Entities::KeyTypeCharSlot)
            return false;
        ak.type = static_cast<AB::Entities::AccountKeyType>(iType);
    }
    else if (filed.compare("count") == 0)
    {
        int iCount = std::atoi(value.c_str());
        if (iCount < 0)
            return false;
        ak.total = static_cast<uint16_t>(iCount);
    }
    else if (filed.compare("status") == 0)
    {
        int iStatus = std::atoi(value.c_str());
        if (iStatus < 0 || iStatus > AB::Entities::KeyStatusBanned)
            return false;
        ak.status = static_cast<AB::Entities::AccountKeyStatus>(iStatus);
    }
    else
        return false;

    return dataClient->Update(ak);
}

void UpdateAccountKeyResource::Render(std::shared_ptr<HttpsServer::Response> response)
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
    auto fieldIt = form.find("field");
    auto valueIt = form.find("value");
    if (uuidIt == form.end() || fieldIt == form.end() || valueIt == form.end())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing field(s)";
    }
    else
    {
        if (!Update((*uuidIt).second, (*fieldIt).second, (*valueIt).second))
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
