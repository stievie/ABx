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

#include "Sessions.h"
#include <sa/StringHash.h>
#include <sa/time.h>

namespace HTTP {

uint32_t Session::sessionLifetime_ = (1000 * 60 * 10);

void Session::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Session"].setClass(kaguya::UserdataMetatable<Session>()
        .addFunction("GetBool", &Session::GetBool)
        .addFunction("GetString", &Session::GetString)
        .addFunction("GetNumber", &Session::GetNumber)
        .addFunction("SetBool", &Session::SetBool)
        .addFunction("SetString", &Session::SetString)
        .addFunction("SetNumber", &Session::SetNumber)
    );
    // clang-format on
}

Session::Session()
{
    Touch();
}

void Session::Touch()
{
    time(&expires_);
    std::tm tm = sa::time::localtime(expires_);
    tm.tm_sec += sessionLifetime_ / 1000;
    expires_ = mktime(&tm);
}

bool Session::GetBool(const std::string& key)
{
    const auto it = values_.find(sa::StringHashRt(key.c_str()));
    if (it == values_.end())
        return false;
    return it->second.GetBool();
}

std::string Session::GetString(const std::string& key)
{
    const auto it = values_.find(sa::StringHashRt(key.c_str()));
    if (it == values_.end())
        return "";
    return it->second.GetString();
}

float Session::GetNumber(const std::string& key)
{
    const auto it = values_.find(sa::StringHashRt(key.c_str()));
    if (it == values_.end())
        return false;
    switch (it->second.GetType())
    {
    case Utils::VariantType::Int:
        return (float)it->second.GetInt();
    case Utils::VariantType::Int64:
        return (float)it->second.GetInt64();
    case Utils::VariantType::Float:
        return it->second.GetFloat();
    default:
        return 0.0f;
    }
}

void Session::SetBool(const std::string& key, bool value)
{
    values_.emplace(sa::StringHashRt(key.c_str()), value);
}

void Session::SetString(const std::string& key, const std::string& value)
{
    values_.emplace(sa::StringHashRt(key.c_str()), value);
}

void Session::SetNumber(const std::string& key, float value)
{
    values_.emplace(sa::StringHashRt(key.c_str()), value);
}

void Sessions::Add(const std::string& id, std::shared_ptr<Session> session)
{
    sessions_[id] = session;
}

std::shared_ptr<Session> Sessions::Get(const std::string& id)
{
    auto it = sessions_.find(id);
    if (it == sessions_.end())
        return std::shared_ptr<Session>();
    time_t now;
    time(&now);
    if ((*it).second->expires_ < now)
    {
        sessions_.erase(it);
        return std::shared_ptr<Session>();
    }
    return (*it).second;
}

}
