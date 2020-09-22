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

#include "LuaContext.h"
#include <abscommon/Xml.h>
#include "Sessions.h"
#include "Cookie.h"
#include "TemplateResource.h"

static void LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

LuaContext::LuaContext(Resources::TemplateResource& resource) :
    resource_(resource)
{
    luaState_.setErrorHandler(LuaErrorHandler);
    HTTP::Cookie::RegisterLua(luaState_);
    HTTP::Cookies::RegisterLua(luaState_);
    HTTP::Session::RegisterLua(luaState_);
    luaState_["session"] = resource.GetSession();
    luaState_["requestCookies"] = resource.GetRequestCookies();
    luaState_["responseCookies"] = resource.GetResponseCookies();
    // The function which writes to the output stream
    luaState_["echo"] = kaguya::function([this](const std::string& value)
    {
        stream_ << value;
    });
    luaState_["escape"] = kaguya::function([](const std::string& value) -> std::string
    {
        return Utils::XML::Escape(value);
    });
    luaState_["getHeader"] = kaguya::function([this](const std::string& key) -> std::string
    {
        const auto& headers = resource_.GetHeaders();
        const auto it = headers.find(key);
        if (it == headers.end())
            return "";
        return (*it).second;
    });
    luaState_["setHeader"] = kaguya::function([this](const std::string& key, const std::string& value)
    {
        auto& headers = resource_.GetHeaders();
        headers.emplace(key, value);
    });
    luaState_["getContentType"] = kaguya::function([this]() -> std::string
    {
        const auto& headers = resource_.GetHeaders();
        const auto it = headers.find("Content-Type");
        if (it == headers.end())
            return "";
        return (*it).second;
    });
    luaState_["setContentType"] = kaguya::function([this](const std::string& value)
    {
        auto& headers = resource_.GetHeaders();
        headers.emplace("Content-Type", value);
    });
}

bool LuaContext::Execute(const std::string& source)
{
    stream_.clear();
    bool result = sa::lpp::Run(luaState_.state(), source);
    if (!result)
    {
        std::istringstream ss(source);
        int line = 0;
        std::string ln;
        while (std::getline(ss, ln))
        {
            std::cout << ++line << ": " << ln << std::endl;
        }
    }
    return result;
}
