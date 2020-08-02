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

void Cookie::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Cookie"].setClass(kaguya::UserdataMetatable<Cookie>()
        .addFunction("GetContent", &Cookie::GetContent)
        .addFunction("SetContent", &Cookie::SetContent)
    );
    // clang-format on
}

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

void Cookies::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Cookies"].setClass(kaguya::UserdataMetatable<Cookies>()
        .addFunction("Get", &Cookies::Get)
        .addFunction("Add", &Cookies::Add)
        .addFunction("Delete", &Cookies::Delete)
    );
    // clang-format on
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
            cookies_.emplace(c[0], std::make_unique<Cookie>(c[1]));
        }
    }
}

void Cookies::Write(SimpleWeb::CaseInsensitiveMultimap& header)
{
    std::vector<std::string> cookies;

    VisitCookies([&cookies](const std::string& name, const Cookie& cookie)
    {
        char buf[64] = { 0 };
        std::string c = name + "=";
        c += cookie.content_ + "; ";
        // Sat, 26-Oct-2019 08:24:53 GMT
        strftime(buf, sizeof(buf), "%a, %e-%b-%G %T GMT", gmtime(&cookie.expires_));
        c += "expires=" + std::string(buf) + "; ";
        c += "path=" + cookie.path_ + "; ";
        if (!cookie.domain_.empty())
            c += "domain=." + cookie.domain_ + "; ";
        switch (cookie.sameSite_)
        {
        case Cookie::SameSite::Lax:
            c += "SameSite=Lax; ";
            break;
        case Cookie::SameSite::Strict:
            c += "SameSite=Strict; ";
            break;
        case Cookie::SameSite::None:
            c += "SameSite=None; ";
            break;
        }
        if (cookie.secure_)
            c += "Secure; ";
        if (cookie.httpOnly_)
            c += "HttpOnly; ";
        c = c.substr(0, c.size() - 2);
        cookies.push_back(c);
        return Iteration::Continue;
    });

    if (cookies.size() != 0)
    {
        std::string content = sa::CombineString<char>(cookies, "");
        header.emplace("Set-Cookie", content);
    }
}

Cookie* Cookies::Add(const std::string& name)
{
    std::unique_ptr<Cookie> cookiePtr = std::make_unique<Cookie>();
    Cookie* result = cookiePtr.get();
    cookies_.emplace(name, std::move(cookiePtr));
    return result;
}

Cookie* Cookies::Get(const std::string& name)
{
    auto it = cookies_.find(name);
    if (it == cookies_.end())
        return nullptr;
    return (*it).second.get();
}

void Cookies::Delete(const std::string& name)
{
    auto it = cookies_.find(name);
    if (it != cookies_.end())
        cookies_.erase(it);
}

}
