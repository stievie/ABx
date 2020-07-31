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


#include "ProfileResource.h"
#include <sa/StringHash.h>
#include <AB/Entities/Account.h>

namespace Resources {

ProfileResource::ProfileResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/profile.lpp";
}

bool ProfileResource::GetContext(LuaContext& objects)
{
    if (!TemplateResource::GetContext(objects))
        return false;

    const std::string& uuid = session_->values_[sa::StringHashRt("account_uuid")].GetString();
    AB::Entities::Account account;
    account.uuid = uuid;
    auto dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Read(account))
        return false;

    kaguya::State& state = objects.GetState();
    std::map<std::string, std::string> xs;
    xs["uuid"] = account.uuid;
    xs["name"] = Utils::XML::Escape(account.name);
    xs["email"] = Utils::XML::Escape(account.email);

    state["account"] = xs;
    return true;
}

void ProfileResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::Normal))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
