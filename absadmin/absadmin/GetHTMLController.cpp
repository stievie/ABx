#include "stdafx.h"
#include "GetHTMLController.h"
#include "Application.h"
#include "Logger.h"
#include "FileUtils.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

bool GetHTMLController::GetObjects(std::map<std::string, ginger::object>& objects)
{
    objects["title"] = "Sample Site";
    return true;
}

void GetHTMLController::MakeRequest(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    GetController::MakeRequest(response, request);

    const std::string& root = Application::Instance->GetRoot();

    auto web_root_path = fs::canonical(root);
    auto path = fs::canonical(web_root_path / request->path);
    // Check if path is within web_root_path
    if (std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
        !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
    {
        LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Trying to access file outside root " << path.string() << std::endl;
        throw std::invalid_argument("path must be within root path");
    }
    if (fs::is_directory(path))
    {
        LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Trying to access a directory " << path.string() << std::endl;
        throw std::invalid_argument("not a file");
    }
    if (Utils::IsHiddenFile(path.string()))
    {
        LOG_ERROR << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << ": "
            << "Trying to access a hidden file " << path.string() << std::endl;
        throw std::invalid_argument("hidden file");
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    header.emplace("Content-Type", "text/html");
    responseCookies_->Write(header);

    auto ifs = std::make_shared<std::ifstream>();
    ifs->open(path.string(), std::ifstream::in | std::ios::ate);

    ifs->seekg(0, std::ios::end);
    size_t size = ifs->tellg();
    std::string buffer(size, ' ');
    ifs->seekg(0);
    ifs->read(&buffer[0], size);

    std::map<std::string, ginger::object> t;
    if (!GetObjects(t))
        return;

    std::stringstream ss;
    ginger::parse(buffer, t, ginger::from_ios(ss));
    header.emplace("Content-Length", std::to_string(ss.str().length()));

    response->write(ss.str(), header);
}
