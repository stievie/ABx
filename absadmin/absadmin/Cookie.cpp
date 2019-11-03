#include "stdafx.h"
#include "Cookie.h"
#include <sa/StringUtils.h>
#include "StringUtils.h"

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
    std::vector<std::string> parts = Utils::Split(cHeader, ";");
    for (const auto& part : parts)
    {
        std::string trimPart = sa::Trim(part);
        std::vector<std::string> c = Utils::Split(trimPart, "=");
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
