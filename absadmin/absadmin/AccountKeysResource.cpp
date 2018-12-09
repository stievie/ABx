#include "stdafx.h"
#include "AccountKeysResource.h"
#include <AB/Entities/AccountKeyList.h>
#include <AB/Entities/AccountKey.h>
#include "StringHash.h"
#include "Application.h"
#include "Version.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "StringUtils.h"
#include "Xml.h"

namespace Resources {

bool AccountKeysResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;

    std::vector<std::map<std::string, ginger::object>> xs;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountKeyList akl;
    if (!dataClient->Read(akl))
        return false;

    for (const auto& a : akl.uuids)
    {
        AB::Entities::AccountKey ak;
        ak.uuid = a;
        if (!dataClient->Read(ak))
            continue;

        xs.push_back({
            { "uuid", ak.uuid },
            { "type", static_cast<int>(ak.type) },
            { "is_type_unknown", ak.type == AB::Entities::KeyTypeUnknown },
            { "is_type_account", ak.type == AB::Entities::KeyTypeAccount },
            { "is_type_charslot", ak.type == AB::Entities::KeyTypeCharSlot },
            { "total", std::to_string(ak.total) },
            { "used", std::to_string(ak.used) },
            { "status", static_cast<int>(ak.status) },
            { "is_status_unknown", ak.status == AB::Entities::KeyStatusUnknown },
            { "is_status_notactivated", ak.status == AB::Entities::KeyStatusNotActivated },
            { "is_status_activated", ak.status == AB::Entities::KeryStatusReadyForUse },
            { "is_status_banned", ak.status == AB::Entities::KeyStatusBanned },
            { "email", Utils::XML::Escape(ak.email) },
            { "description", Utils::XML::Escape(ak.description) },
        });
    }

    const uuids::uuid guid = uuids::uuid_system_generator{}();
    objects["new_id"] = guid.to_string();

    objects["accountkeys"] = xs;
    return true;
}

AccountKeysResource::AccountKeysResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/accountkeys.html";

    styles_.insert(styles_.begin(), "vendors/datatables.net-bs/css/dataTables.bootstrap.min.css");
    styles_.insert(styles_.begin(), "vendors/datatables.net-buttons-bs/css/buttons.bootstrap.min.css");
    styles_.insert(styles_.begin(), "vendors/datatables.net-fixedheader-bs/css/fixedHeader.bootstrap.min.css");
    styles_.insert(styles_.begin(), "vendors/datatables.net-responsive-bs/css/responsive.bootstrap.min.css");
    styles_.insert(styles_.begin(), "vendors/datatables.net-scroller-bs/css/scroller.bootstrap.min.css");

    // Datatables
    footerScripts_.push_back("vendors/datatables.net/js/jquery.dataTables.min.js");
    footerScripts_.push_back("vendors/datatables.net-bs/js/dataTables.bootstrap.min.js");
    footerScripts_.push_back("vendors/datatables.net-buttons/js/dataTables.buttons.min.js");
    footerScripts_.push_back("vendors/datatables.net-buttons-bs/js/buttons.bootstrap.min.js");
    footerScripts_.push_back("vendors/datatables.net-buttons/js/buttons.flash.min.js");
    footerScripts_.push_back("vendors/datatables.net-buttons/js/buttons.html5.min.js");
    footerScripts_.push_back("vendors/datatables.net-buttons/js/buttons.print.min.js");
    footerScripts_.push_back("vendors/datatables.net-fixedheader/js/dataTables.fixedHeader.min.js");
    footerScripts_.push_back("vendors/datatables.net-keytable/js/dataTables.keyTable.min.js");
    footerScripts_.push_back("vendors/datatables.net-responsive/js/dataTables.responsive.min.js");
    footerScripts_.push_back("vendors/datatables.net-responsive-bs/js/responsive.bootstrap.js");
    footerScripts_.push_back("vendors/datatables.net-scroller/js/dataTables.scroller.min.js");
}

void AccountKeysResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}