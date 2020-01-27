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

namespace Resources {

class Resource
{
protected:
    std::shared_ptr<HttpsServer::Request> request_;
    std::unique_ptr<HTTP::Cookies> requestCookies_;
    std::unique_ptr<HTTP::Cookies> responseCookies_;
    std::shared_ptr<HTTP::Session> session_;
    void Redirect(std::shared_ptr<HttpsServer::Response> response, const std::string& url);
    bool IsAllowed(AB::Entities::AccountType minType);
public:
    explicit Resource(std::shared_ptr<HttpsServer::Request> request);
    Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    virtual ~Resource() = default;

    virtual void Render(std::shared_ptr<HttpsServer::Response> response) = 0;
};

}
