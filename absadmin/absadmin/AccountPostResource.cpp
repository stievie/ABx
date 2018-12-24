#include "stdafx.h"
#include "AccountPostResource.h"
#include "Application.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"

namespace Resources {

void AccountPostResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
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
    if (uuidIt == form.end())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing UUID field";
    }
    else
    {
        std::string uuid = (*uuidIt).second;
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
            auto typeIt = form.find("account_type");
            if (typeIt != form.end())
            {
                int t = std::atoi((*typeIt).second.c_str());
                account.type = static_cast<AB::Entities::AccountType>(t);
            }
            auto statusIt = form.find("account_status");
            if (statusIt != form.end())
            {
                int s = std::atoi((*statusIt).second.c_str());
                account.status = static_cast<AB::Entities::AccountStatus>(s);
            }

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
