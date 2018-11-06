#include "stdafx.h"
#include "CreateKeyResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"
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
    {
        const uuids::uuid guid = uuids::uuid_system_generator{}();
        ak.uuid = guid.to_string();
    }
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
    if (!IsAllowed(AB::Entities::AccountTypeGod))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(Utils::GetFileExt(".json")));
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
