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


#include "AccountKeysResource.h"
#include <AB/Entities/AccountKeyList.h>
#include <AB/Entities/AccountKey.h>
#include "Application.h"
#include "Version.h"

namespace Resources {

bool AccountKeysResource::GetContext(LuaContext& objects)
{
    if (!TemplateResource::GetContext(objects))
        return false;

    auto* dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountKeyList akl;
    if (!dataClient->Read(akl))
        return false;

    kaguya::State& state = objects.GetState();
    state["accountkeys"] = kaguya::NewTable();
    int i = 0;
    for (const std::string& a : akl.uuids)
    {
        AB::Entities::AccountKey ak;
        ak.uuid = a;
        if (!dataClient->Read(ak))
            continue;

        state["accountkeys"][++i] = kaguya::TableData{
            { "uuid", ak.uuid },
            { "type", static_cast<int>(ak.type) },
            { "is_type_unknown", ak.type == AB::Entities::KeyTypeUnknown },
            { "is_type_account", ak.type == AB::Entities::KeyTypeAccount },
            { "is_type_charslot", ak.type == AB::Entities::KeyTypeCharSlot },
            { "is_type_chestslot", ak.type == AB::Entities::KeyTypeChestSlots },
            { "total", std::to_string(ak.total) },
            { "used", std::to_string(ak.used) },
            { "status", static_cast<int>(ak.status) },
            { "is_status_unknown", ak.status == AB::Entities::KeyStatusUnknown },
            { "is_status_notactivated", ak.status == AB::Entities::KeyStatusNotActivated },
            { "is_status_activated", ak.status == AB::Entities::KeryStatusReadyForUse },
            { "is_status_banned", ak.status == AB::Entities::KeyStatusBanned },
            { "email", Utils::XML::Escape(ak.email) },
            { "description", Utils::XML::Escape(ak.description) }
        };
    }
    state["new_id"] = Utils::Uuid::New();
    return true;
}

AccountKeysResource::AccountKeysResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/accountkeys.lpp";

    styles_.insert(styles_.begin(), "vendors/css/footable.bootstrap.min.css");
    footerScripts_.push_back("vendors/js/footable.min.js");
}

void AccountKeysResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
