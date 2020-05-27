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


#include "Cookie.h"
#include <sa/StringTempl.h>

namespace HTTP {

Cookie::Cookie() :
    path_("/"),
    httpOnly_(false)
{
    time(&expires_);
    struct tm* tm = localtime(&expires_);
    tm->tm_year++;
    expires_ = mktime(tm);
}

Cookie::Cookie(const std::string& content) :
    path_("/"),
    httpOnly_(false),
    content_(content)
{
    time(&expires_);
    struct tm* tm = localtime(&expires_);
    tm->tm_year++;
    expires_ = mktime(tm);
}

Cookies::Cookies(const HttpsServer::Request& request)
{
    // Cookie: CONSENT=WP.272d88; 1P_JAR=2018-10-26-06; NID=144=UE-02yrDztg9wZze3sLgWzEs4YR713X4Onjzj0uEzpIUWlymHKJ9ZcJsrbbWjZMci0c8wQAFVH6pTT-hZQvcbsr77wKG5mGfVkS2OH1bXfFdIlEzBMzOZrESgpNSJnHZdoyZdiGd3tTd6MpvtDJo4X34hgMD1ZPUsKJvVQr_pLI
    const SimpleWeb::CaseInsensitiveMultimap& header = request.header;
    auto it = header.find("Cookie");
    if (it == header.end())
        return;
    auto cHeader = (*it).second;
    std::vector<std::string> parts = sa::Split(cHeader, ";");
    for (const auto& part : parts)
    {
        std::string trimPart = sa::Trim(part);
        std::vector<std::string> c = sa::Split(trimPart, "=");
        if (c.size() == 2)
        {
            cookies_.emplace(c[0], c[1]);
        }
    }
}

void Cookies::Write(SimpleWeb::CaseInsensitiveMultimap& header)
{
    std::vector<std::string> cookies;
    for (const auto& cookie : cookies_)
    {
        char buf[64] = { 0 };
        std::string c = cookie.first + "=";
        c += cookie.second.content_ + "; ";
        // Sat, 26-Oct-2019 08:24:53 GMT
        strftime(buf, sizeof(buf), "%a, %e-%b-%G %T GMT", gmtime(&cookie.second.expires_));
        c += "expires=" + std::string(buf) + "; ";
        c += "path=" + cookie.second.path_ + "; ";
        if (!cookie.second.domain_.empty())
            c += "domain=." + cookie.second.domain_ + "; ";
        if (cookie.second.httpOnly_)
            c += "HttpOnly; ";

        c = c.substr(0, c.size() - 2);
        cookies.push_back(c);
    }
    if (cookies.size() != 0)
    {
        std::string content = sa::CombineString<char>(cookies, "");
        header.emplace("Set-Cookie", content);
    }
}

void Cookies::Add(const std::string& name, const Cookie& cookie)
{
    cookies_.emplace(name, cookie);
}

Cookie* Cookies::Get(const std::string& name)
{
    auto it = cookies_.find(name);
    if (it == cookies_.end())
        return nullptr;
    return &(*it).second;
}

}
