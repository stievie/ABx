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

#pragma once

#include "Cookie.h"
#include "Sessions.h"
#include <AB/Entities/Account.h>
#include "Servers.h"
#include <sa/Noncopyable.h>
#include <optional>

namespace Resources {

class Resource
{
    NON_COPYABLE(Resource)
    NON_MOVEABLE(Resource)
protected:
    static std::string GetETagValue(const std::string& content);
    std::shared_ptr<HttpsServer::Request> request_;
    SimpleWeb::CaseInsensitiveMultimap header_;
    std::unique_ptr<HTTP::Cookies> requestCookies_;
    std::unique_ptr<HTTP::Cookies> responseCookies_;
    std::shared_ptr<HTTP::Session> session_;
    SimpleWeb::CaseInsensitiveMultimap formValues_;
    SimpleWeb::CaseInsensitiveMultimap queryValues_;
    void Redirect(std::shared_ptr<HttpsServer::Response> response, const std::string& url);
    bool IsAllowed(AB::Entities::AccountType minType);
    virtual void Send(const std::string& content, std::shared_ptr<HttpsServer::Response> response);
    std::string GetRequestHeader(const std::string& key);
public:
    explicit Resource(std::shared_ptr<HttpsServer::Request> request);
    virtual ~Resource() = default;

    virtual void Render(std::shared_ptr<HttpsServer::Response> response) = 0;
    SimpleWeb::CaseInsensitiveMultimap& GetHeaders() { return header_; }
    const SimpleWeb::CaseInsensitiveMultimap& GetHeaders() const { return header_; }
    std::optional<std::string> GetFormField(const std::string& key);
    std::optional<std::string> GetQueryValue(const std::string& key);
};

}
