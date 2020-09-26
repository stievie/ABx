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

#include "SpawnResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include <sa/ScopeGuard.h>

namespace Resources {

bool SpawnResource::Spawn(const std::string& uuid)
{
    if (uuid.empty() || uuids::uuid(uuid).nil())
        return false;

    auto msgClient = GetSubsystem<Net::MessageClient>();
    if (!msgClient)
        return false;
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::Spawn;
    msg.SetBodyString(uuid);
    return msgClient->Write(msg);
}

void SpawnResource::Render(std::shared_ptr<HttpsServer::Response> response)
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

    auto uuidIt = GetFormField("uuid");
    if (!uuidIt.has_value())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing UUID field";
        return;
    }
    if (!Spawn(uuidIt.value()))
    {
        obj["status"] = "Failed";
        obj["message"] = "Failed";
        return;
    }
    obj["status"] = "OK";
}

}
