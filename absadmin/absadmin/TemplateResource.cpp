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
#include "TemplateResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include "Version.h"
#include <sa/StringHash.h>
#include <AB/Entities/Account.h>

namespace Resources {

bool TemplateResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    assert(Application::Instance);
    objects["title"] = Utils::XML::Escape(Application::Instance->GetServerName());
    objects["copy_year"] = SERVER_YEAR;
    auto it = session_->values_.find(sa::StringHashRt("username"));
    if (it != session_->values_.end())
    {
        objects["user"] = Utils::XML::Escape((*it).second.GetString());
    }
    else
        objects["user"] = "";
    auto accIt = session_->values_.find(sa::StringHashRt("account_type"));
    AB::Entities::AccountType accType = AB::Entities::AccountTypeUnknown;
    if (accIt != session_->values_.end())
        accType = static_cast<AB::Entities::AccountType>((*accIt).second.GetInt());
    objects["is_user"] = accType >= AB::Entities::AccountTypeNormal;
    objects["is_tutor"] = accType >= AB::Entities::AccountTypeTutor;
    objects["is_sentutor"] = accType >= AB::Entities::AccountTypeSeniorTutor;
    objects["is_gm"] = accType >= AB::Entities::AccountTypeGamemaster;
    objects["is_god"] = accType >= AB::Entities::AccountTypeGod;

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
    styles_.push_back("vendors/nprogress/nprogress.css");
    styles_.push_back("css/custom.less");

    headerScripts_.push_back("vendors/jquery/dist/jquery.min.js");
    footerScripts_.push_back("vendors/bootstrap/dist/js/bootstrap.min.js");
    footerScripts_.push_back("vendors/nprogress/nprogress.js");
    footerScripts_.push_back("js/common.js");
}

void TemplateResource::LoadTemplates(std::string& result)
{
    std::string body = GetTemplateFile(template_);
    if (body.empty())
        return;

    std::ifstream ifs(body, std::ifstream::in | std::ios::binary);
    if (!ifs)
    {
        LOG_ERROR << "Unable to open file " << body << std::endl;
        throw std::invalid_argument("Error opening file");
    }

    ifs.seekg(0, std::ios::end);
    size_t size = ifs.tellg();
    size_t currSize = result.length();
    result.resize(currSize + size);
    ifs.seekg(0, std::ios::beg);
    ifs.read(&result[currSize], size);
}

std::string TemplateResource::GetTemplateFile(const std::string& templ)
{
    if (templ.empty())
        return "";

    const std::string& root = Application::Instance->GetRoot();

    auto web_root_path = fs::canonical(root);
    auto path = fs::canonical(web_root_path / templ);
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

    return path.string();
}

void TemplateResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    AB_PROFILE;
    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    // Don't cache templates
    header.emplace("Cache-Control", "no-cache, no-store, must-revalidate");
    responseCookies_->Write(header);

    std::string buffer;
    try
    {
        LoadTemplates(buffer);
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR << "Template Load Error: " << ex.what() << std::endl;
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
            "Internal Server Error " + request_->path);
        return;
    }

    std::map<std::string, ginger::object> t;
    std::string templFile = GetTemplateFile(template_);
    t["__template__"] = templFile;
    t["__template_path__"] = Utils::ExtractFileDir(templFile);

    if (!GetObjects(t))
    {
        LOG_ERROR << "Failed to get objects" << std::endl;
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request_->path);
        return;
    }

    try
    {
        std::stringstream ss;
        ginger::parse(buffer, t, ginger::from_ios(ss));
        ss.seekg(0, std::ios::end);
        size_t ssize = ss.tellg();
        ss.seekg(0, std::ios::beg);
        header.emplace("Content-Type", contT->Get(Utils::GetFileExt(template_)));
        header.emplace("Content-Length", std::to_string(ssize));
        response->write(ss, header);
    }
    catch (const ginger::parse_error& ex)
    {
        LOG_ERROR << "Parse Error: " << ex.long_error() << std::endl;
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
            "Internal Server Error " + request_->path);
    }
}

}
