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


#include "ProfilePostResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include <sa/StringHash.h>

namespace Resources {

void ProfilePostResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::Normal))
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
    auto emailIt = form.find("email");
    if (emailIt == form.end())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing Email field";
    }
    else
    {
        std::string uuid = session_->values_[sa::StringHashRt("account_uuid")].GetString();
        AB::Entities::Account account;
        account.uuid = uuid;
        auto dataClient = GetSubsystem<IO::DataClient>();
        if (!dataClient->Read(account))
        {
            obj["status"] = "Failed";
            obj["message"] = "Invalid Account";
        }
        else
        {
            account.email = (*emailIt).second;
            if (dataClient->Update(account))
                obj["status"] = "OK";
            else
            {
                obj["status"] = "Failed";
                obj["message"] = "Update failed";
            }
        }
    }

    response->write(obj.dump(), header);
}

}
