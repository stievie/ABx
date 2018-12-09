#include "stdafx.h"
#include "AccountsResource.h"
#include "StringHash.h"
#include "Application.h"
#include "Version.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "StringUtils.h"
#include <AB/Entities/AccountList.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Game.h>
#include "UuidUtils.h"
#include "Xml.h"

namespace Resources {

bool AccountsResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;

    return true;
}

AccountsResource::AccountsResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/accounts.html";

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

void AccountsResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
