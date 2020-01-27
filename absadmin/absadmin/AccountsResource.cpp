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

#include "stdafx.h"
#include "AccountsResource.h"
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
