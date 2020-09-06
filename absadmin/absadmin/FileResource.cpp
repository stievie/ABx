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


#include "FileResource.h"
#include "Application.h"
#include "ContentTypes.h"

namespace Resources {

void FileResource::SendFileRange(std::shared_ptr<HttpsServer::Response> response,
    const std::string& path, const sa::http::range& range,
    bool multipart, const std::string& boundary)
{
    auto ifs = std::make_shared<std::ifstream>();
    ifs->open(path, std::ifstream::in | std::ios::binary | std::ios::ate);
    ASSERT(ifs);

    auto fileSize = (long)ifs->tellg();
    size_t start = range.start;
    size_t end = (range.end != 0) ? range.end : (size_t)fileSize;
    ASSERT(end > start);
    size_t length = end - start;

    ifs->seekg(start, std::ios::beg);

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    responseCookies_->Write(header);
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(Utils::GetFileExt(request_->path)));
    header.emplace("Cache-Control", "max-age=2592000, public");             //30days (60sec * 60min * 24hours * 30days)

    if (multipart)
    {
        response->write("--" + boundary);
        response->write("Content-Type: application/octet-stream\n");
        response->write("Content-Range: " + std::to_string(range.start) + "-" +
            std::to_string(range.end) + "/" + std::to_string(fileSize) + "\n");
        response->write("\n");
    }

    struct FileServer
    {
        static void ReadAndSend(const std::shared_ptr<HttpsServer::Response>& response,
            const std::shared_ptr<std::ifstream>& ifs, size_t remaining)
        {
            size_t chunkSize = std::min<size_t>(131072u, remaining);
            std::vector<char> buffer;
            buffer.resize(chunkSize);
            std::streamsize read_length;
            if ((read_length = ifs->read(&buffer[0], static_cast<std::streamsize>(buffer.size())).gcount()) > 0)
            {
                response->write(&buffer[0], read_length);
                if (read_length == static_cast<std::streamsize>(buffer.size()))
                {
                    response->send([response, ifs, remaining, read_length](const SimpleWeb::error_code& ec)
                    {
                        if (!ec)
                        {
                            size_t newRemaining = remaining - read_length;
                            if (newRemaining > 0)
                                ReadAndSend(response, ifs, newRemaining);
                        }
                        else
                            LOG_ERROR << "Connection interrupted " << ec.default_error_condition().value() << " " <<
                            ec.default_error_condition().message() << std::endl;
                    });
                }
            }
        }
    };
    FileServer::ReadAndSend(response, ifs, length);
}

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


        std::ifstream ifs(path.string(), std::ifstream::in | std::ios::binary | std::ios::ate);
        if (!ifs)
            throw std::invalid_argument("could not read file");

        auto fileSize = ifs.tellg();

        const auto rangeHeaderIt = request_->header.find("Range");
        sa::http::ranges ranges;
        if (rangeHeaderIt == request_->header.end())
        {
            ranges.push_back({ 0, 0, 0 });
        }
        else
        {
            if (!sa::http::parse_ranges(fileSize, rangeHeaderIt->second, ranges))
            {
                response->write(SimpleWeb::StatusCode::client_error_range_not_satisfiable,
                    "Range Not Satisfiable");
                return;
            }
        }

        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Range_requests
        const bool multipart = ranges.size() > 1;
        const bool isRange = !sa::http::is_full_range(fileSize, ranges[0]);
        // Multipart not supported
        if (multipart)
        {
            LOG_WARNING << "TODO: Multipart not supported" << std::endl;
            response->write(SimpleWeb::StatusCode::client_error_range_not_satisfiable,
                "Range Not Satisfiable");
            return;
        }

        const std::string boundary = "3d6b6a416f9b5";

        SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();

        responseCookies_->Write(header);
        auto contT = GetSubsystem<ContentTypes>();

        if (isRange && !multipart)
        {
            // Single part of a file
            header.emplace("Content-Type", contT->Get(Utils::GetFileExt(request_->path)));
            header.emplace("Cache-Control", "max-age=2592000, public");             //30days (60sec * 60min * 24hours * 30days)
            header.emplace("Content-Length", std::to_string(ranges[0].length));
            header.emplace("Content-Range", std::to_string(ranges[0].start) + "-" +
                std::to_string(ranges[0].end) + "/" + std::to_string(fileSize));
            response->write(SimpleWeb::StatusCode::success_partial_content, header);
        }
        else if (isRange && multipart)
        {
            // Multiple parts of a file in one response -> multipart message
            header.emplace("Content-Type", "multipart/byteranges; boundary=" + boundary);
            header.emplace("Content-Length", std::to_string(sa::http::content_length(ranges)));
            response->write(SimpleWeb::StatusCode::success_partial_content, header);
        }
        else
        {
            // Whole file
            header.emplace("Content-Type", contT->Get(Utils::GetFileExt(request_->path)));
            header.emplace("Cache-Control", "max-age=2592000, public");             //30days (60sec * 60min * 24hours * 30days)
            header.emplace("Content-Length", std::to_string(fileSize));
            response->write(SimpleWeb::StatusCode::success_ok, header);
        }

        SendFileRange(response, path.string(), ranges[0], multipart, boundary);

#if 0
        if (multipart)
            response->write("--" + boundary + "--\n");
#endif
    }
    catch (const std::exception&)
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found " + request_->path);
    }
}

}
