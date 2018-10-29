#include "stdafx.h"
#include "ServicesResource.h"
#include "StringHash.h"
#include "Application.h"
#include "Version.h"
#include "Subsystems.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>
#include "DataClient.h"

namespace Resources {

bool ServicesResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;

    std::vector<std::map<std::string, ginger::object>> xs;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::ServiceList sl;
    if (!dataClient->Read(sl))
        return false;
    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        xs.push_back({
            { "uuid", s.uuid },
            { "name", s.name },
            { "file", s.file },
            { "host", s.host },
            { "load", std::to_string(s.load) },
            { "location", s.location },
            { "path", s.path },
            { "port", s.port },
            { "run_time", s.runTime },
            { "start_time", s.startTime },
            { "stop_time", s.stopTime },
            { "status", s.status },
            { "online", (bool)(s.status == AB::Entities::ServiceStatusOnline) },
            { "type", s.type },
        });
    }
    objects["services"] = xs;

    return true;
}

ServicesResource::ServicesResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/services.html";

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

void ServicesResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
        TemplateResource::Render(response);
    else
        Redirect(response, "/");
}

}
