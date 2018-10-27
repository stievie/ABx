#include "stdafx.h"
#include "TemplateResource.h"
#include "Application.h"
#include "Logger.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "Profiler.h"

namespace Resources {

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

    std::stringstream ss;
    try
    {
        ginger::parse(buffer, t, ginger::from_ios(ss));
        ss.seekg(0, std::ios::end);
        size_t ssize = ss.tellg();
        ss.seekg(0, std::ios::beg);
        header.emplace("Content-Length", std::to_string(ssize));
        response->write(ss, header);
    }
    catch (const ginger::parse_error& ex)
    {
        LOG_ERROR << "Parse Error(" << ex.line1() << "):" << ex.long_error() << std::endl;
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
            "Parse Error " + request_->path);
    }
}

}
