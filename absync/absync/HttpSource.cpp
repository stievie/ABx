/**
 * Copyright 2020 Stefan Ascher
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

#include "HttpSource.h"
#include <sa/http_range.h>
#include <AB/CommonConfig.h>
#include <sa/http_status.h>
#include <sa/time.h>
#include <numeric>

namespace Sync {

HttpSource::HttpSource(const std::string& host, uint16_t port, httplib::Headers headers) :
    host_(host),
    port_(port),
    headers_(headers)
{
}

httplib::SSLClient* HttpSource::GetHttpClient()
{
    if (!client_)
    {
        if (!host_.empty() && port_ != 0)
            client_ = std::make_unique<httplib::SSLClient>(host_, port_);
        if (client_)
            client_->enable_server_certificate_verification(HTTP_CLIENT_VERIFY_SSL);
    }
    if (client_)
        return client_.get();
    return nullptr;
}

std::vector<char> HttpSource::GetChunk(const std::string& filename, size_t start, size_t length)
{
    auto* client = GetHttpClient();
    if (client == nullptr)
        return {};

    httplib::Headers header(headers_);
    if (start != 0 || length != 0)
    {
        const sa::http::range range{ start, start + length, (ssize_t)length };
        header.emplace("Range", sa::http::to_string(range));
    }

    std::string remoteName(filename);
    if (remoteName.front() != '/')
        remoteName = "/" + remoteName;
    std::vector<char> result;
    sa::time::timer timer;
    int64_t elapsed = 0;
    auto res = client->Get(remoteName.c_str(), header, [&](const char* data, uint64_t size) -> bool
    {
        result.reserve(result.size() + size);
        std::copy(data, data + size, std::back_inserter(result));
        // Division by zero is something really nasty!
        int64_t millis = std::max<int64_t>(1, timer.elapsed_millis());
        float bps = (float)size / (float)millis * 1000.0f;
        elapsed += millis;

        if (elapsed >= 20)
        {
            if (onDownloadProgress_)
            {
                if (!onDownloadProgress_(static_cast<size_t>(bps)))
                    return false;
            }
            elapsed = 0;
        }
        timer.restart();
        return true;
    });

    if (!res)
    {
        switch (res.error())
        {
        case httplib::Connection:
            Error("Connection");
            break;
        case httplib::BindIPAddress:
            Error("Bind IP Address");
            break;
        case httplib::Read:
            Error("Read");
            break;
        case httplib::Write:
            Error("Write");
            break;
        case httplib::ExceedRedirectCount:
            Error("Exceed redirect count");
            break;
        case httplib::Canceled:
            Error("Canceled");
            break;
        case httplib::SSLConnection:
            Error("SSL Connection");
            break;
        case httplib::SSLServerVerification:
            Error("SSL Server verification");
            break;
        default:
            break;
        }
        return {};
    }
    if (res->status == 200 || res->status == 206)
        return result;
    Error(sa::http::status_message(res->status));
    return {};
}

}
