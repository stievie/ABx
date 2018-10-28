#include "stdafx.h"
#include "TemplateResource.h"
#include "Application.h"
#include "Logger.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "Profiler.h"
#include "Version.h"
#include "StringHash.h"

namespace Resources {

bool TemplateResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    objects["title"] = "ABS Admin";
    objects["copy_year"] = SERVER_YEAR;
    auto it = session_->values_.find(Utils::StringHashRt("username"));
    if (it != session_->values_.end())
        objects["user"] = (*it).second.GetString();
    else
        objects["user"] = "";

    std::vector<std::map<std::string, ginger::object>> s;
    for (const auto& f : styles_)
    {
        s.push_back({
            { "url", f }
        });
    }
    objects["styles"] = s;

    std::vector<std::map<std::string, ginger::object>> hs;
    for (const auto& f : headerScripts_)
    {
        hs.push_back({
            { "url", f }
        });
    }
    objects["header_scripts"] = hs;

    std::vector<std::map<std::string, ginger::object>> fs;
    for (const auto& f : footerScripts_)
    {
        fs.push_back({
            { "url", f }
        });
    }
    objects["footer_scripts"] = fs;

    return true;
}

TemplateResource::TemplateResource(std::shared_ptr<HttpsServer::Request> request) :
    Resource(request)
{
    styles_.push_back("vendors/bootstrap/dist/css/bootstrap.min.css");
    styles_.push_back("vendors/font-awesome/css/font-awesome.min.css");
    styles_.push_back("css/custom.min.css");
    styles_.push_back("css/icon-trill.css");
    footerScripts_.push_back("vendors/jquery/dist/jquery.min.js");
}

void TemplateResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    const std::string& root = Application::Instance->GetRoot();

    auto web_root_path = fs::canonical(root);
    auto path = fs::canonical(web_root_path / GetTemplate());
    if (fs::is_directory(path))
    {
        LOG_ERROR << request_->remote_endpoint_address() << ":" << request_->remote_endpoint_port() << ": "
            << "Trying to access a directory " << path.string() << std::endl;
        throw std::invalid_argument("not a file");
    }
    if (Utils::IsHiddenFile(path.string()))
    {
        LOG_ERROR << request_->remote_endpoint_address() << ":" << request_->remote_endpoint_port() << ": "
            << "Trying to access a hidden file " << path.string() << std::endl;
        throw std::invalid_argument("hidden file");
    }

    AB_PROFILE;
    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(Utils::GetFileExt(".html")));
    // Don't cache templates
    header.emplace("Cache-Control", "no-cache, no-store, must-revalidate");
    responseCookies_->Write(header);

    auto ifs = std::make_shared<std::ifstream>();
    ifs->open(path.string(), std::ifstream::in | std::ios::ate);

    ifs->seekg(0, std::ios::end);
    size_t size = ifs->tellg();
    std::string buffer;
    buffer.resize(size);
    ifs->seekg(0);
    ifs->read(&buffer[0], size);

    std::map<std::string, ginger::object> t;
    if (!GetObjects(t))
        return;

    try
    {
        std::stringstream ss;
        ginger::parse(buffer, t, ginger::from_ios(ss));
        ss.seekg(0, std::ios::end);
        size_t ssize = ss.tellg();
        ss.seekg(0, std::ios::beg);
        header.emplace("Content-Length", std::to_string(ssize));
        response->write(ss, header);
    }
    catch (const ginger::parse_error& ex)
    {
        LOG_ERROR << "Parse Error: " << ex.long_error() << std::endl;
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
            "Parse Error " + request_->path);
    }
}

}
