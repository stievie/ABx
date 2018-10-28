#include "stdafx.h"
#include "IndexResource.h"
#include "Application.h"
#include "Logger.h"
#include "FileUtils.h"
#include "Variant.h"
#include "StringHash.h"
#include "Version.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/AccountList.h>
#include <AB/Entities/CharacterList.h>
#include "Subsystems.h"
#include "DataClient.h"

namespace Resources {

bool IndexResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (!loggedIn)
        return true;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountList al;
    if (!dataClient->Read(al))
        return false;
    objects["total_accounts"] = al.uuids.size();

    AB::Entities::CharacterList cl;
    if (!dataClient->Read(cl))
        return false;
    objects["total_chars"] = cl.uuids.size();

    return true;
}

std::string IndexResource::GetTemplate()
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
        return "../templates/dashboard.html";
    return "../templates/login.html";
}

IndexResource::IndexResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (!loggedIn)
        return;

    styles_.push_back("vendors/nprogress/nprogress.css");
    styles_.push_back("vendors/iCheck/skins/flat/green.css");
    styles_.push_back("vendors/bootstrap-progressbar/css/bootstrap-progressbar-3.3.4.min.css");
    styles_.push_back("vendors/jqvmap/dist/jqvmap.min.css");
    styles_.push_back("vendors/bootstrap-daterangepicker/daterangepicker.css");

    // Bootstrap
    footerScripts_.push_back("vendors/bootstrap/dist/js/bootstrap.min.js");
    footerScripts_.push_back("vendors/fastclick/lib/fastclick.js");
    footerScripts_.push_back("vendors/nprogress/nprogress.js");
    footerScripts_.push_back("vendors/Chart.js/dist/Chart.min.js");
    footerScripts_.push_back("vendors/gauge.js/dist/gauge.min.js");
    footerScripts_.push_back("vendors/bootstrap-progressbar/bootstrap-progressbar.min.js");
    footerScripts_.push_back("vendors/iCheck/icheck.min.js");
    footerScripts_.push_back("vendors/skycons/skycons.js");
    footerScripts_.push_back("vendors/Flot/jquery.flot.js");
    footerScripts_.push_back("vendors/Flot/jquery.flot.pie.js");
    footerScripts_.push_back("vendors/Flot/jquery.flot.time.js");
    footerScripts_.push_back("vendors/Flot/jquery.flot.stack.js");
    footerScripts_.push_back("vendors/Flot/jquery.flot.resize.js");
    footerScripts_.push_back("vendors/flot.orderbars/js/jquery.flot.orderBars.js");
    footerScripts_.push_back("vendors/flot-spline/js/jquery.flot.spline.min.js");
    footerScripts_.push_back("vendors/flot.curvedlines/curvedLines.js");
    footerScripts_.push_back("vendors/DateJS/build/date.js");
    footerScripts_.push_back("vendors/jqvmap/dist/jquery.vmap.js");
    footerScripts_.push_back("vendors/jqvmap/dist/maps/jquery.vmap.world.js");
    footerScripts_.push_back("vendors/jqvmap/examples/js/jquery.vmap.sampledata.js");
    footerScripts_.push_back("vendors/moment/min/moment.min.js");
    footerScripts_.push_back("vendors/bootstrap-daterangepicker/daterangepicker.js");
    footerScripts_.push_back("js/custom.js");
}

}
