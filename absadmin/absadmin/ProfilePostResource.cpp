#include "stdafx.h"
#include "ProfilePostResource.h"
#include "Application.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"

namespace Resources {

void ProfilePostResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeNormal))
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
        std::string uuid = session_->values_[Utils::StringHashRt("account_uuid")].GetString();
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
