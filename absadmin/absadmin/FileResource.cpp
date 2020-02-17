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
#include "FileResource.h"
#include "Application.h"
#include "ContentTypes.h"

namespace Resources {

void FileResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    const std::string& root = Application::Instance->GetRoot();

    try
    {
        auto web_root_path = fs::canonical(root);
        auto path = fs::canonical(Utils::AddSlash(root) + request_->path);
        // Check if path is within web_root_path
        if (std::distance(web_root_path.begin(), web_root_path.end()) > std::distance(path.begin(), path.end()) ||
            !std::equal(web_root_path.begin(), web_root_path.end(), path.begin()))
        {
            LOG_ERROR << request_->remote_endpoint_address() << ":" << request_->remote_endpoint_port() << ": "
                << "Trying to access file outside root " << path.string() << std::endl;
            throw std::invalid_argument("path must be within root path");
        }
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

        SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
        responseCookies_->Write(header);
        auto contT = GetSubsystem<ContentTypes>();
        header.emplace("Content-Type", contT->Get(Utils::GetFileExt(request_->path)));
        header.emplace("Cache-Control", "max-age=2592000, public");             //30days (60sec * 60min * 24hours * 30days)

        auto ifs = std::make_shared<std::ifstream>();
        ifs->open(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);

        if (*ifs)
        {
            auto length = ifs->tellg();
            ifs->seekg(0, std::ios::beg);

            header.emplace("Content-Length", to_string(length));
            response->write(header);

            // Trick to define a recursive function within this scope (for example purposes)
            class FileServer
            {
            public:
                static void read_and_send(const std::shared_ptr<HttpsServer::Response> &response,
                    const std::shared_ptr<std::ifstream> &ifs)
                {
                    // Read and send 128 KB at a time
                    std::vector<char> buffer(131072);
                    std::streamsize read_length;
                    if ((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) > 0)
                    {
                        response->write(&buffer[0], read_length);
                        if (read_length == static_cast<std::streamsize>(buffer.size()))
                        {
                            response->send([response, ifs](const SimpleWeb::error_code &ec)
                            {
                                if (!ec)
                                    read_and_send(response, ifs);
                                else
                                    LOG_ERROR << "Connection interrupted" << std::endl;
                            });
                        }
                    }
                }
            };
            FileServer::read_and_send(response, ifs);
        }
        else
            throw std::invalid_argument("could not read file");
    }
    catch (const std::exception&)
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request_->path);
    }
}

}
