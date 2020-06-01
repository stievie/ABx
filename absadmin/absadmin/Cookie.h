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

#include <string>
#include <map>
#include "Servers.h"

namespace HTTP {

class Cookie
{
public:
    enum class SameSite
    {
        Lax = 0,
        Strict,
        None
    };
    Cookie();
    Cookie(const std::string& content);
    ~Cookie() = default;

    std::string domain_;
    std::string path_;
    time_t expires_;
    bool httpOnly_;
    SameSite sameSite_{ SameSite::Lax };
    bool secure_{ false };
    std::string content_;
};

class Cookies
{
private:
    std::map<std::string, Cookie> cookies_;
public:
    Cookies() = default;
    explicit Cookies(const HttpsServer::Request& request);
    void Write(SimpleWeb::CaseInsensitiveMultimap& header);
    void Add(const std::string& name, const Cookie& cookie);
    Cookie* Get(const std::string& name);
};

}
